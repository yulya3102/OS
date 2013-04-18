List = require("list")

read_fd = function(fd, from)
    fd:seek("set", from)
    return fd:read()
end

sockets = {}

Socket = {}

Socket.new = function(name)
    print("creating socket ", name)
    local fd = io.tmpfile()
    sockets[name] = { fd = fd, connections = {}, number = 0 }
    return fd
end

Socket.connect = function(name)
    print("connecting to socket ", name)
    local socket = sockets[name]
    socket.number = socket.number + 1
    socket.connections[socket.number] = 0
    return socket.number
end

Socket.read = function(args)
    local name = args.name
    print("reading from socket ", name)
    local id = args.id
    local socket = sockets[name]
    if socket == nil then
        print("socket doesn't exist")
        return nil
    end
    local result = read_fd(socket.fd, socket.connections[id])    
    if result ~= nil then
        socket.connections[id] = socket.connections[id] + string.len(result)
    end
    return result
end

Socket.write = function(args)
    local name = args.name
    print("writing to socket ", name)
    local data = args.data
    local socket = sockets[name]
    if socket == nil then
        print("socket doesn't exist")
        return
    end
    socket.fd:write(data)
end

syscall_tags = { exit = 0, create = 1, connect = 2, read = 3, write = 4 }

context = function(tag, args, cont)
    return { tag = tag, args = args, cont = cont }
end

process1 = function(arg)
    return context(syscall_tags.create, "socket1", function(fd)
        return context(syscall_tags.write, { name = "socket1", data = "data_in_socket1" }, function(arg)
            return context(syscall_tags.write, { name = "socket1", data = "more_data_in_socket1" }, function(arg)
                return context(syscall_tags.exit, nil, nil)
            end)
        end)
    end)
end

process2 = function(arg)
    return context(syscall_tags.connect, "socket1", function(socket_id)
        return context(syscall_tags.read, { name = "socket1", id = socket_id }, function(string)
            print(string)
            return context(syscall_tags.read, { name = "socket1", id = socket_id }, function(string)
                print(string)
                return context(syscall_tags.exit, nil, nil)
            end)
        end)
    end)
end

process = function(proc, arg)
    return { proc = proc, arg = arg }
end

kernel = function()
    local runnable = List.new()
    local unrunnable = List.new()
    List.pushright(runnable, process(process1, nil))
    List.pushright(runnable, process(process2, nil))
    while not List.is_empty(runnable) or not List.is_empty(unrunnable) do
        -- run next runnable
        -- in runnable - processes
        local proc = not List.is_empty(runnable) and List.popleft(runnable) or nil
        if proc ~= nil then
            local context = proc.proc(proc.arg)
            if context.tag == syscall_tags.exit then
                print("exit process")
            elseif context.tag == syscall_tags.create then
                proc = process(context.cont, Socket.new(context.args))
                List.pushright(runnable, proc)
            elseif context.tag == syscall_tags.connect then
                proc = process(context.cont, Socket.connect(context.args))
                List.pushright(runnable, proc)
            elseif context.tag == syscall_tags.read then
                List.pushright(unrunnable, context)
            elseif context.tag == syscall_tags.write then
                proc = process(context.cont, Socket.write(context.args))
                List.pushright(runnable, proc)
            else
                print("unknown syscall")
            end
        end
        -- check every unrunnable
        -- in unrunnable - contexts
        local n = List.size(unrunnable)
        for i = 1, n, 1 do
            local context = List.popleft(unrunnable)
            if context.tag ~= syscall_tags.read then
                -- how did it get there?
                print("error: sycall_tag ", context.tag, " in unrunnable")
            else
                local result = Socket.read(context.args)
                if result ~= nil then
                    List.pushright(runnable, process(context.cont, result))
                else
                    List.pushright(unrunnable, context)
                end
            end
        end
    end
end

kernel()
