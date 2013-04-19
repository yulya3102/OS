local List = {}

List.new = function()
	return { first = 0, last = -1 }
end

List.get = function(list, value)
    -- O(n) :(((
    if List.is_empty(list) then
        return nil
    end
    local stack = List.new()
    repeat
        local item = List.popleft(list)
        if item == value then
            if List.is_empty(stack) then
                return item
            end
            repeat
                List.pushleft(list, List.popright(stack))
            until List.is_empty(stack)
            return item
        end
        List.pushright(stack, item)
    until List.is_empty(list)
    if List.is_empty(stack) then
        return nil
    end
    repeat
        List.pushleft(list, List.popright(stack))
    until List.is_empty(stack)
    return nil
end

function List.pushleft (list, value)
	local first = list.first - 1
	list.first = first
	list[first] = value
end

function List.pushright (list, value)
	local last = list.last + 1
	list.last = last
	list[last] = value
end

function List.popleft (list)
	local first = list.first
	if first > list.last then error("list is empty") end
	local value = list[first]
	list[first] = nil        -- to allow garbage collection
	list.first = first + 1
	return value
end

function List.popright (list)
	local last = list.last
	if list.first > last then error("list is empty") end
	local value = list[last]
	list[last] = nil         -- to allow garbage collection
	list.last = last - 1
	return value
end

function List.is_empty (list)
	return list.first > list.last
end

function List.size (list)
	return list.last - list.first + 1
end

return List
