let _ = require('lodash')

function stat_group(name,opts) {
    let {category,description} = opts || {}  
    category = `STATCAT_${category || 'Advanced'}`
    description = '<js stat group>'
    let group_name = `STATGROUP_${name}`, group_description = description 
    return {
        stat: (name, opts) => {
            let {description, default_enable, should_clear_everyframe, type, cycle} = opts || {}
            name = `STAT_${name}`
            description = description || '<js stat>'
            if (default_enable == undefined) {
                default_enable = true
            }
            if (should_clear_everyframe == undefined) {
                should_clear_everyframe = false
            }
            if (type == undefined) {
                type = 'ST_int64'
            }
            if (cycle == undefined) {
                cycle = false
            }

            let ref = JavascriptLibrary.NewStat(
              name, description, 
              group_name, category, group_description,
              default_enable, should_clear_everyframe, type, cycle)

            let i = {
                clear: x => ref.AddMessage('Clear')
            }

            let msg_func = {
                ST_int64: (...args) => ref.AddMessage_int(...args),
                ST_double: (...args) => ref.AddMessage_float(...args)
            }[type]

            if (msg_func) {
                i = _.extend(i, {
                    inc: (x=1) => msg_func('Add',x,cycle),
                    dec: (x=1) => msg_func('Subtract',x,cycle),
                    set: x => msg_func('Set',x,cycle)                     
                })
            }   

            if (cycle) {
                i = _.extend(i, {
                    measure: fn => {
                        ref.AddMessage('CycleScopeStart');
                        try {
                            fn()
                        } catch (e) {
                            throw e
                        } finally {
                            ref.AddMessage('CycleScopeEnd')
                        }
                    }
                })
            } 

            return i  
        }
    }
}

module.exports = stat_group