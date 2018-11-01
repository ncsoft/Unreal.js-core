(function (global) {
    "use strict";

    let _ = require('lodash')
    function isClass (thing) {
        return typeof thing === 'function' && !thing.hasOwnProperty('arguments')
    }
    module.exports = function () {
        let RE_class = /\s*class\s+(\w+)(\s+\/\*([^\*]*)\*\/)?(\s+extends\s+([^\s\{]+))?/
        let RE_func = /(\w+)\s*\(([^.)]*)\)\s*(\/\*([^\*]*)\*\/)?.*/
        function register(target, klass, template) {
            target = target || {}
            let bindings = []

            let splits = RE_class.exec(template) || (isClass(template) ? [null, template.name] : null)
            if (!splits) throw "Invalid class definition"

            let orgClassName = splits[1]
            
            let properties = []
            let classFlags = (splits[3] || "").split('+').map((x) => x.trim())

            function refactored(x) {
                let m = /\s*(\w+)\s*(\/\*([^\*]*)\*\/)?\s*/.exec(x)
                if (m) {
                    let arr = (m[3] || '').split('+').map((x) => x.trim())
                    let type = arr.pop()                        
                    let is_array = false                    
                    let is_subclass = false                    
                    let is_map = false
                    if (/\[\]$/.test(type)) {
                        is_array = true
                        type = type.substr(0, type.length - 2)
                    }
                    if (/\<\>$/.test(type)) {
                        is_subclass = true
                        type = type.substr(0, type.length - 2)
                    }
                    if(/\{\}$/.test(type)) {
                        is_map = true
                        let kv = (m[3] || '').split('::').map(x => x.trim());
                        let tv = _.map(kv, t => t.split('+').map(x => x.trim()));
                        type = tv[0].pop() + '::' + tv[1].pop();           
                        type = type.substr(0, type.length - 2)                        
                        arr = _.concat(tv[0],  tv[1]);

                    }
                    if (_.isFunction(target[type])) {
                        let src = String(target[type])
                        let e = /function (\w+)\(/.exec(src)
                        if (e) {
                            type = e[1]
                        }
                    }

                    if (type) {
                        return {
                            Name: m[1],
                            Type: type,
                            Decorators: arr,
                            IsSubclass: is_subclass,
                            IsArray: is_array,
                            IsMap: is_map
                        }
                    } else {
                        return null
                    }
                } else {
                    return null
                }
            }

            let proxy = {}
            _(Object.getOwnPropertyNames(template.prototype)).filter((name) => {
                let c = Object.getOwnPropertyDescriptor(template.prototype, name);
                return (c.get || c.set) == undefined;
            })
            .forEach((k) => {
                if (k == "properties") {
                    let func = String(template.prototype[k])
                    func = func.substr(func.indexOf('{')+1)
                    func = func.substr(0, func.lastIndexOf('}'))
                    func = _.compact(func.split('\n').map((l) => l.trim())).map((l) => {
                        if (l.indexOf("this.") != 0) return
                        l = l.substr(5)
                        return refactored(l)
                    })
                    properties = func
                }
                else if (k != 'constructor') {
                    //@note : assume that ufunction would not added at rebinding time.
                    proxy[k] = template.prototype[k];
                }
            })

            let thePackage = JavascriptLibrary.CreatePackage(null,'/Script/Javascript')

            if (_.includes(classFlags, "Struct")) {
                RebindStructProperties({
                    SelfStruct: klass,
                    Functions: proxy,
                    Properties: properties
                });
            }
            else {
                RebindClassProperties({
                    SelfClass: klass,
                    Functions: proxy,
                    Properties: properties
                });
            }
        }

        return register;
    }
}(this))
