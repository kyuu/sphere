class Clock {
    constructor() {
        _started = false
        _time = 0
        _ticksAtStart = null
    }

    function start() {
        if (!_started) {
            _ticksAtStart = GetTicks()
            _started = true
        }
    }

    function stop() {
        if (_started) {
            _time += GetTicks() - _ticksAtStart
            _ticksAtStart = null
            _started = false
        }
    }

    function reset() {
        _time = 0
        if (_started) {
            _ticksAtStart = GetTicks()
        }
    }

    function getTicks() {
        return _time + (_started ? (GetTicks() - _ticksAtStart) : 0)
    }

    function setTicks(ticks) {
        _time = ticks
        if (_started) {
            _ticksAtStart = GetTicks()
        }
    }

    function getTime() {
        local ms = _time + (_started ? (GetTicks() - _ticksAtStart) : 0)
        return {
            second = (ms / 1000) % 60
            minute = (ms / 60000) % 60
            hour   = (ms / 3600000)
        }
    }

    function setTime(hour, minute, second) {
        _time = hour * 3600000 + minute * 60000 + second * 1000
        if (_started) {
            _ticksAtStart = GetTicks()
        }
    }

    function getTimeString() {
        local time = getTime()
        local s = (time.second < 10 ? "0" : "") + time.second
        local m = (time.minute < 10 ? "0" : "") + time.minute
        local h = (time.hour   < 10 ? "0" : "") + time.hour
        return h + ":" + m + ":" + s
    }

    _started = null
    _time = null
    _ticksAtStart = null
}
