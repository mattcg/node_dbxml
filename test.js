var	dbxml = require('./dbxml'),
	sys = require('sys'),
	puts = sys.puts;

puts('Creating environment...');

var e = dbxml.createEnv("/tmp/dbxml");

puts('Created environment.');