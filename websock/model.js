var DB = require('./db').DB;

var User = DB.Model.extend({
   tableName: 'WebUsers',
   idAttribute: 'userId',
});

module.exports = {
   User: User
};
