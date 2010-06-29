import Options
from os import unlink, symlink, popen
from os.path import exists

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
	opt.tool_options('compiler_cxx')

def configure(conf):
	conf.check_tool('compiler_cxx')
	conf.check_tool('node_addon')
	conf.check(lib='dbxml-2.5',  uselib_store='dbxml-2.5', mandatory=True)
	conf.check(lib='xerces-c',  uselib_store='xerces-c', mandatory=True)
	conf.check(lib='xqilla',  uselib_store='xqilla', mandatory=True)
	conf.check(lib='db_cxx-4.8',  uselib_store='db_cxx-4.8', mandatory=True)

	# BERKELEY DB XML must be installed to /usr/local
	# i.e.
	# cd dbxml-2.5.16
	# ./buildall.sh --prefix=/usr/local

def build(bld):
	obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
	obj.target = 'binding'
	obj.source = "binding.cc"
	obj.uselib = "dbxml-2.5 xerces-c xqilla db_cxx-4.8"

def shutdown():
	# HACK to get binding.node out of build directory.
	# better way to do this?
	if Options.commands['clean']:
		if exists('binding.node'): unlink('binding.node')
	else:
		if exists('build/default/binding.node') and not exists('binding.node'):
			symlink('build/default/binding.node', 'binding.node')
