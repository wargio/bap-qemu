sh4_ss = ss.source_set()
sh4_ss.add(files(
  'cpu.c',
  'gdbstub.c',
  'helper.c',
  'op_helper.c',
  'translate.c',
))

if get_option('tracewrap')
  sh4_ss.add(files('trace_helper.c'))
endif

sh4_softmmu_ss = ss.source_set()
sh4_softmmu_ss.add(files('monitor.c'))

target_arch += {'sh4': sh4_ss}
target_softmmu_arch += {'sh4': sh4_softmmu_ss}
