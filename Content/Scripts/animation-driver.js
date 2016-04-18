(function (global) {
    "use strict"
    function currentTime() {
        return global.$time
    }
    
    function $default_access(target, key) {
        var h = target["Set" + key]
        if (typeof h == 'function') return h.bind(target)
    }

    function animationDriver() {        
        var alive = true
        var running = false
        var animations = []
        
        class Anim {
            constructor(target,meta,anim) {
                var $access = meta.$access || $default_access
                var tracks = this.tracks = []
                for (var k in anim) {
                    var fn = anim[k]                
                    var h = $access(target,k)

                    if (typeof h == 'function') {
                        tracks.push([fn,h])
                    } else {
                        console.error(`No such track{${k}}`)
                    }
                }
                this.duration = meta.duration || 0.25
                this.delay = (meta.delay || 0) + currentTime();
                this.loop = meta.loop || 1
                this.completed = meta.completed || function() {}
                this.interrupted = meta.interrupted || function() {}
                this.started = currentTime()
                this.tracks = tracks
                this.added = false
                this.warm = meta.warm
            }
            tick(t) {
                if(this.delay) {
                    if(this.delay - t > 0)
                        return
                    this.started = currentTime()
                    this.delay = 0
                }

                var alpha = (t - this.started) / this.duration
                var lap = Math.floor(alpha)
                
                var shouldQuit = (this.loop != 0 && lap >= this.loop)
                if (shouldQuit) {
                    alpha = 1
                } else {
                    alpha -= lap
                }
                                
                this.tracks.forEach(track => {
                    let fn = track[0]
                    let h = track[1]
                    let value = fn(alpha)
                    if (value != undefined) {
                        h.call(null,value)
                    }
                })
                if (shouldQuit) {
                    this.remove()
                }
            }
            add() {
                if (this.added) return
                this.added = true
                animations.push(this)
                if (this.warm) {
                    this.tracks.forEach(track => {
                        let fn = track[0]
                        let h = track[1]
                        let value = fn(0)
                        if (value != undefined) {
                            h.call(null,value)
                        }
                    })
                }
            }
            remove() {
                if (!this.added) return
                this.added = false
                animations.splice(animations.indexOf(this), 1)
                if (animations.length == 0) {
                    stop()
                }
            }
        }
        function applyAnim(target, meta, anim) {          
            let I = new Anim(target,meta,anim)      
            return new Promise((resolve, reject) => {
                I.add()
                run(I, resolve)
            })
            .then(_=> {
                I.completed()
            }, _=>{
                I.interrupted()                
           }).catch(_=> {
           })
        }

        function loop(I, resolve) {
            if (!I.added || !running){
                resolve()
                return
            } 
            var t = currentTime()
            animations.forEach(anim => anim.tick(t))
            process.nextTick(_=> loop(I, resolve))
        }

        function run(I, resolve) {
            if (!alive) {
                resolve()
                return
            }
            running = true
            loop(I, resolve)
        }

        function stop() {
            running = false
        }

        return {
            apply: applyAnim,
            destroy: _ => {
                alive = false
                animations.length = 0
                stop()
            },
            is_alive: _ => alive
        }
    }

    module.exports = animationDriver
})(this)