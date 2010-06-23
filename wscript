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

	conf.env.append_value('CCFLAGS',  '-I/Users/mattcg/Source/dbxml-2.5.16/install/include')
	conf.env.append_value('CXXFLAGS',  '-I/Users/mattcg/Source/dbxml-2.5.16/install/include')

def build(bld):
	obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
	obj.target = 'binding'
	obj.source = "binding.cc"

def shutdown():
	# HACK to get binding.node out of build directory.
	# better way to do this?
	if Options.commands['clean']:
		if exists('binding.node'): unlink('binding.node')
	else:
		if exists('build/default/binding.node') and not exists('binding.node'):
			symlink('build/default/binding.node', 'binding.node')
