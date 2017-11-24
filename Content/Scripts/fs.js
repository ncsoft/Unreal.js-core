var filePrefix = "file://"
function prefixTrim(path) {
    if (path.startsWith(filePrefix))
        return decodeURI(path.substr(filePrefix.length))
    return path
}

module.exports = {
    readFileSync : function (path,encoding) {
        path = prefixTrim(path)
        var text = Context.ReadScriptFile(path);
        if (encoding == 'utf8') return text;
        return {
            toString: function () {
                return text;
            }
        }
    },

    existsSync : function (path) {
        path = prefixTrim(path)
        return JavascriptLibrary.FileExists(path)
    }
};