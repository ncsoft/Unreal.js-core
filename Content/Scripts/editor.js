(function (global) {
	"use strict"

    function getTimestampString() {
        let now = new Date()
        return `${now.getFullYear()}-${String(now.getMonth() + 1).padStart(2, '0')}-${String(now.getDay()).padStart(2, '0')} ${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}:${String(now.getSeconds()).padStart(2, '0')}.${now.getMilliseconds()}`
    }

	let has_module = false
	try { if (eval("module")) { has_module = true } } catch (e) {}
	if (has_module) {
        let _ = require('lodash')
        let prerequisites = []
        let extensions = []
        let root_path = Root.GetDir('GameContent') + 'Scripts'

		function read_dir(dir) {
			let out = Root.ReadDirectory(dir)
            if (out.$) {
                let prerequisite_items = _.filter(out.OutItems, (item) => !item.bIsDirectory && /^((?!node_modules).)*$/.test(item.Name) && /prerequisite[^\.]*\.js$/.test(item.Name))
                prerequisites = prerequisites.concat(prerequisite_items.map((item) => item.Name.substr(root_path.length + 1)))
				let extension_items = _.filter(out.OutItems, (item) => !item.bIsDirectory && /^((?!node_modules).)*$/.test(item.Name) && /extension[^\.]*\.js$/.test(item.Name))
                extensions = extensions.concat(extension_items.map((item) => item.Name.substr(root_path.length + 1)))
				out.OutItems.forEach((item) => {
					if (item.bIsDirectory) {
						read_dir(item.Name)
					}
				})
			}
		}

		read_dir(root_path)

		function spawn(moduleCategory, modulePath) {
            try {
                console.log(`[${getTimestampString()}]: (editor.js) Importing ${moduleCategory} module: '${modulePath}'`)
				return require(modulePath)()
			}
			catch (e) {
				console.error(String(e))
				return function () {}
			}
		}

        function main() {
            let bye_prerequisites = _.filter(prerequisites.map(prerequisite => spawn('prerequisite', prerequisite)), _.isFunction)
		    let byes = _.filter(extensions.map(extension => spawn('extension', extension)), _.isFunction)

			return function () {
                _.concat(byes, bye_prerequisites).forEach(byeFunc => byeFunc())
			}
		}

		module.exports = () => {
            try {
                let cleanup = main()

				global.$$exit = cleanup
		
				return () => cleanup()
			} catch (e) {
				console.error(String(e))
				return () => {}
			}
		}
	} else {
		global.$$exit = function() {}
		global.$exit = function () {
			global.$$exit()
		}
		Context.WriteDTS(Context.Paths[0] + 'typings/ue.d.ts')
		Context.WriteAliases(Context.Paths[0] + 'aliases.js')

		Context.RunFile('aliases.js')
		Context.RunFile('polyfill/unrealengine.js')
		Context.RunFile('polyfill/timers.js')

		require('devrequire')('editor')
	}
})(this)
