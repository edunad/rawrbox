function ui:create(name, parent)
    if name == "frame" then return ui:createFrame(parent) end
    if name == "button" then return ui:createButton(parent) end
    if name == "label" then return ui:createLabel(parent) end
    if name == "progress_bar" then return ui:createProgressBar(parent) end
    if name == "input" then return ui:createInput(parent) end
    if name == "group" then return ui:createGroup(parent) end
    if name == "image" then return ui:createImage(parent) end
    if name == "graph" then return ui:createGraph(parent) end

    error("Unknown element type ", name)
    return nil
end
