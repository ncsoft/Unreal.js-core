"use strict"

function makeWindowTimer(target) {
    var freeSymbols = []
    function sym_alloc() {
        if (freeSymbols.length) {
            return freeSymbols.pop()
        } else {
            return Symbol()
        }
    }
    function sym_free(sym) {
        freeSymbols.push(sym)
    }

    var FastPriorityQueue = require('./FastPriorityQueue')
    var Queue = new FastPriorityQueue(function (a, b) {
        return a.T < b.T;
    })
    var currentTime = 0
    var frame = 0
    var timers = {}    
    target.setTimeout = function (handler, timeout) {
        var timerId = sym_alloc()
        var t = timers[timerId] = {
            T: currentTime + timeout,
            handler: handler,
            frame: frame,
            active: true,
            interval: 0
        }
        Queue.add(t)
        return timerId
    }
    target.clearTimeout = function (timerId) {
        var t = timers[timerId]
        if (t) {
            t.active = false
            delete timers[timerId]
            sym_free(timerId)
        }
    }
    target.setInterval = function (handler, interval) {
        var timerId = sym_alloc()
        var t = timers[timerId] = {
            T: currentTime + interval,
            handler: handler,
            frame: frame,
            active: true,
            interval: interval
        }
        Queue.add(t)
        return timerId
    }
    target.clearInterval = target.clearTimeout

    return function (elapsedTime) {
        currentTime += elapsedTime
        while (Queue.size) {
            var y = Queue.peek()
            if (y.T >= currentTime || y.frame == frame) break

            Queue.poll()
            if (y.active) {
                y.handler()                
                if (y.interval) {
                    y.T += y.interval
                    Queue.add(y)
                }
            }
        }
        frame++
    }
}

(function (target) {
    if (Root == undefined || Root.OnTick == undefined) return
    
    var timerLoop = makeWindowTimer(target);

    var current_time = 0
    target.$time = 0

    var nextTicks = []

    function flushTicks() {
        nextTicks.push(null)
        while (true) {
            var x = nextTicks.shift()
            if (!x) break
            x()
        }
    }    
    
    var root = function (elapsedTime) {        
        flushTicks()
        
        current_time += elapsedTime
        target.$time = current_time
        
        timerLoop(elapsedTime | 0)
    }

    target.process = {
        nextTick: function (fn) {
            nextTicks.push(fn)
        },        
        argv: [],
        argc: 0,
        platform: 'UnrealJS',
        env: {}
    }

    Root.OnTick.Add(root)
})(global)