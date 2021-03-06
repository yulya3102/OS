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
    sockets[name] = { fd = fd, connections = {} }
    return fd
end

Socket.connect = function(name, pid)
    print("connecting to socket ", name)
    local socket = sockets[name]
    socket.connections[pid] = 0
end

Socket.read = function(name, pid)
    print("reading from socket ", name)
    local socket = sockets[name]
    if socket == nil then
        print("socket doesn't exist")
        return nil
    end
    local result = read_fd(socket.fd, socket.connections[pid])    
    if result ~= nil then
        socket.connections[pid] = socket.connections[pid] + string.len(result)
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
            return context(syscall_tags.connect, "socket1", function(arg)
                return context(syscall_tags.read, "socket1", function(string)
                    print(string)
                    return context(syscall_tags.write, { name = "socket1", data = "more_data_in_socket1" }, function(arg)
                        return context(syscall_tags.exit, nil, nil)
                    end)
                end)
            end)
        end)
    end)
end

process2 = function(arg)
    return context(syscall_tags.connect, "socket1", function(arg)
        return context(syscall_tags.read, "socket1", function(string)
            print(string)
            return context(syscall_tags.read, "socket1", function(string)
                print(string)
                return context(syscall_tags.exit, nil, nil)
            end)
        end)
    end)
end

process = function(proc, arg)
    return { proc = proc, arg = arg }
end

last_pid = 0

next_pid = function()
    last_pid = last_pid + 1
    return last_pid
end

kernel = function()
    local processes = {}
    local runnable = List.new()
    local unrunnable = List.new()
    List.pushright(runnable, next_pid())
    processes[last_pid] = process(process1, nil)
    List.pushright(runnable, next_pid())
    processes[last_pid] = process(process2, nil)
    List.pushright(runnable, next_pid())
    processes[last_pid] = process(process2, nil)
    while not List.is_empty(runnable) or not List.is_empty(unrunnable) do
        -- run next runnable
        -- in runnable - processes
        local pid = not List.is_empty(runnable) and List.popleft(runnable) or nil
        local updated_socket = nil
        if pid ~= nil then
            local proc = processes[pid]
            local context = proc.proc(proc.arg)
            if context.tag == syscall_tags.exit then
                print("exit process")
            elseif context.tag == syscall_tags.create then
                proc = process(context.cont, Socket.new(context.args))
                List.pushright(runnable, pid)
                processes[pid] = proc
            elseif context.tag == syscall_tags.connect then
                proc = process(context.cont, Socket.connect(context.args, pid))
                List.pushright(runnable, pid)
                processes[pid] = proc
            elseif context.tag == syscall_tags.read then
                local result = Socket.read(context.args, pid)
                if result == nil then
                    List.pushright(unrunnable, pid)
                    processes[pid] = context
                else
                    proc = process(context.cont, result)
                    List.pushright(runnable, pid)
                    processes[pid] = proc
                end
            elseif context.tag == syscall_tags.write then
                proc = process(context.cont, Socket.write(context.args))
                List.pushright(runnable, pid)
                processes[pid] = proc
                updated_socket = context.args.name
            else
                print("unknown syscall")
            end
        end

        if updated_socket ~= nil then
            local socket = sockets[updated_socket]
            for connected_pid, pos in pairs(socket.connections) do
                -- if process with pid = connected_pid is in unrunnable then run it
                local pid = List.get(unrunnable, connected_pid)
                if pid ~= nil then
                    print("unrunnable process can be updated", pid)
                    local context = processes[pid]
                    if context.tag ~= syscall_tags.read then
                        -- how did it get there?
                        print("error: sycall_tag", context.tag, "in unrunnable")
                    else
                        local result = Socket.read(context.args, pid)
                        List.pushright(runnable, pid)
                        processes[pid] = process(context.cont, result)
                    end
                end
            end
        end
    end
end

kernel()
