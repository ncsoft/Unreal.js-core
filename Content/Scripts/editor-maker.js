function box_extension(where) {
    return function (opts) {
        var menux = new JavascriptUIExtender
        var builders = {}
        var exts = []
        for (var k in opts) {
            var v = opts[k]
            var position = v.position || 'After'
            exts.push({ExtensionHook : k, HookPosition : position})

            var fn = v.builder
            builders[k] = fn
        }

        menux[where] = exts
        menux.OnHook.Add( function (hook) {
            var fn = builders[hook]
            if (fn) {
                fn(menux)
            }
        })
        return menux
    }
}

if (!global.group) {
    global.group = JavascriptEditorLibrary.GetGroup('Root').AddGroup('Unreal.js').AddGroup('Extension demo')
} 

function MakeTab(opts,tab_fn,del_fn) {
    opts = opts || {}

    var tab = new JavascriptEditorTab
    tab.TabId = opts.TabId || 'TestJSTab'
    tab.Role = opts.Role || 'NomadTab'
    tab.DisplayName = opts.DisplayName || '안녕하세요!'
    tab.Group = opts.Group || global.group
    tab.OnSpawnTab.Add(tab_fn)
    if (del_fn) {
        tab.OnCloseTab.Add(del_fn)    
    }
    return tab
}

module.exports = {
    menu : box_extension('MenuExtensions'),
    toolbar : box_extension('ToolbarExtensions'),
    tab : MakeTab,
    tabSpawner : function (opts,main) {
        let $tabs = global.$tabs = global.$tabs || {}
        let $inner = global.$tabinner = global.$tabinner || []        
        let $fns = global.$tabfns = global.$tabfns || {}
        const id = opts.TabId
        
        $fns[id] = main
        let opened = $tabs[id]
        
        function create_inner(fn,where) {
            let child
            try {
                child = fn()    
            } catch (e) {
                console.error(String(e),e.stack)
                child = new TextBlock()
                child.SetText(`ERROR:${String(e)}`)
            }
            $inner.push(child)
            where.AddChild(child)
        }
        if (opened) {
            opened.forEach(open => {
                let old = SizeBox.C(open).GetChildAt(0)
                old.destroy && old.destroy()
                $inner.splice($inner.indexOf(old),1)
                SizeBox.C(open).RemoveChildAt(0)
                create_inner(main,open)                
            })
            return _ => {}
        }        
        
        opened = $tabs[id] = []
        
        let tab = MakeTab(opts, (context) => {
            let widget = new SizeBox()
            let fn = $fns[id]            
            opened.push(widget)
            create_inner(fn,widget)
            
            return widget
        },widget => {
            let content = widget.GetContentSlot().Content
            content.destroy()
            
            $inner.splice($inner.indexOf(widget.GetChildAt(0)),1)
            opened.splice(opened.indexOf(widget),1)     
            widget.RemoveChildAt(0);
        })
        tab.Commit()
        
        opened.$spawner = tab
    },
    commands : function make_commands(opts) {
        var commands = new JavascriptUICommands

        opts = opts || {}
        commands.ContextName = opts.name || opts.Name || ''
        commands.ContextDesc = opts.description || opts.Description || 'context description'
        commands.ContextNameParent = opts.parent || opts.Parent || ''
        commands.StyleSetName = opts.styleset || opts.Styleset || 'None'

        var org_cmds = opts.commands || {}

        var cmds = []
        for (var k in org_cmds) {
            var v = org_cmds[k]
            cmds.push({
                Id : k,
                FriendlyName : v.name || v.Name,
                Description : v.description || v.Description,
                ActionType : v.type || v.Type || 'Button'
            })
        }
        commands.Commands = cmds
        commands.OnExecuteAction.Add( function (action) {
            var fn = org_cmds[action]
            fn && fn.execute && fn.execute()
            fn && fn.Execute && fn.Execute()
        })

        "OnCanExecuteAction/enabled OnIsActionChecked/checked OnIsActionButtonVisible/visible".split(' ')
        .forEach( function (v) {
            var xy = v.split('/')
            var x = xy[0]
            var y = xy[1]
            commands[x].Add( function (action) {
                var fn = org_cmds[action]
                if (fn && fn.query) {
                    return fn.query(y)
                }
                if (fn && fn.Query) {
                    return fn.Query(y)
                }
                return true
            })
        })
        return commands
    },
    editor : function make_editor(opts) {
        var editor = new JavascriptAssetEditorToolkit
        editor.ToolkitFName = 'jseditor'
        editor.BaseToolkitFName = 'jseditor_base'
        editor.ToolkitName = 'jseditor toolkit'
        editor.WorldCentricTabPrefix = 'jseditor'

        editor.Layout = JSON.stringify(opts.layout)

        if (opts.tabs != undefined) {
            editor.Tabs = opts.tabs
        }
        if (opts.commands != undefined) {
            editor.Commands = opts.commands
        }
        if (opts.menu != undefined) {
            editor.MenuExtender = opts.menu
        }
        if (opts.toolbar != undefined) {
            editor.ToolbarExtender = opts.toolbar
        }
        return editor
    }
}
