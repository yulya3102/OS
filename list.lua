local List = {}

List.new = function()
	return { first = 0, last = -1 }
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

--[[function List.popleft_with_tag (list, tag)
	local stack = List.new()
	local item = nil
	local result = nil
	repeat
		item = not List.is_empty(list) and List.popleft(list) or nil
		if item ~= nil and item.tag == tag then
			result = item
			break
		end
		if item ~= nil then Lust.pushright(stack, item) end
	until item == nil
	while not List.is_empty(stack) do
		List.pushleft(list, List.popright(stack))
	end
	return result
end]]

return List
