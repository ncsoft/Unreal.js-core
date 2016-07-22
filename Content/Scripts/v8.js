module.exports = {
    setFlagsFromString: function (string) {
        JavascriptLibrary.V8_SetFlagsFromString(string)
    },
    setIdleTaskBudget: function (budgetInSeconds) {
        JavascriptLibrary.V8_SetIdleTaskBudget(budgetInSeconds)
    }
};