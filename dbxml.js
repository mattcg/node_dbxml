var binding = require("./binding");

var Env = binding.Env;

exports.createEnv = function (env_home) {
  var e = new Env;

  e.createEnv(env_home);

  return e;
};