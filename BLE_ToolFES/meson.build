# Correr con: meson builddir/ --cross-file cross-file.txt
# Luego:      cd builddir 
#             ninja
# Flash:      pyocd flash -t nrf51 ./builddir/minimal.hex
# pyocd flash -t nrf51 nrf51_ble_test_project.hex

# Al parecer aqui se deben definir todos los source files que ocupa el programa.

project('minimal', 'c')

assert(meson.is_cross_build(), 'Cross compile environment expected')

linker_script = 'ble_app_uart_c_gcc_nrf51.ld'
sdk_root = 'E:\Respaldo\Escritorio\ProyectosBelen\SDK_NRF'
softdevice = files(join_paths(sdk_root, 'components/softdevice/s130/hex/s130_nrf51_2.0.1_softdevice.hex'))
objcopy = find_program('arm-none-eabi-objcopy')
mergehex = find_program('mergehex')

#Estos flags estan sacandos directamente del makefile de ejemplo desde el cual me baso.
#Sinceramente, aun no se de donde se obtienen los nombres que se ocupan aqui.
#Update: Estos son los defines que desbloquean ciertas secciones del codigo
# ejemplo, en algun archivo existe un "#ifdef XXXX", esa seccion de codigo se desbloquea
# si aqui esta definido ese "XXXX" 
# Lo que aun no entiendo es por que llevan una "D" al comienzo, será de Directorio (?, 
# refiriendose a que es un define de los directorios, o simplemente D de "define", 
# pero los archivos con D son los defines de los archivos.
add_global_arguments(
    '-DBOARD_PCA10028',
    '-DSOFTDEVICE_PRESENT',
    '-DNRF51',
    '-DS130',
    '-DBLE_STACK_SUPPORT_REQD',
    '-DSWI_DISABLE0',
    '-DBSP_UART_SUPPORT',
    '-DNRF51822', 
    '-DNRF_SD_BLE_API_VERSION=2',
    '-mcpu=cortex-m0',
    '-mabi=aapcs',
    '-mthumb', 
    '-Wall', 
    '-O3', 
    '-g3',
    '-mfloat-abi=soft',
    '-ffunction-sections', 
    '-fdata-sections', 
    '-fno-strict-aliasing',
    '-fno-builtin', 
    '--short-enums',
    # '-DBSP_DEFINES_ONLY',
    language : ['c', 'cpp']
)

add_global_link_arguments(
    '-T' + join_paths(meson.source_root(), linker_script),
    '-L' + join_paths(sdk_root, 'components/toolchain/gcc'),
    '-mcpu=cortex-m0',
    '-mthumb',
    '-mabi=aapcs',
    '-Wl,--gc-sections', # Esto dumpea las sesiones no usadas
    '-lc',
    '-lnosys',
    '--specs=nano.specs', # usa la nueva libreria en la version nano.
    '-lm',
    language : ['c', 'cpp']
)

# Los sources son todos los archivos .c que el programa ocupa
sources = files([
  'main.c',
  'ble_lib/imu_lib/imu_tests.c',
  # 'servo_lib/utils.c',
  # 'servo_lib/servo_tests.c',
  'ble_lib/fes_lib/fes_uuids.c',
  'ble_lib/fes_lib/fes_ble.c',
  'ble_lib/fes_lib/fes_tests.c',
  'ble_lib/imu_lib/imu_uuids.c',
  'ble_lib/imu_lib/imu_ble.c',
  'ble_lib/ble_connection.c',
  #'custom_ble_services/ble_cus.c',
  join_paths(sdk_root,'components/libraries/log/src/nrf_log_backend_serial.c'),
  join_paths(sdk_root,'components/libraries/log/src/nrf_log_frontend.c'),
  join_paths(sdk_root,'components/libraries/util/app_error.c'),
  join_paths(sdk_root,'components/libraries/button/app_button.c'),
  join_paths(sdk_root,'components/libraries/util/app_error_weak.c'),
  join_paths(sdk_root,'components/libraries/fifo/app_fifo.c'),
  join_paths(sdk_root,'components/libraries/timer/app_timer.c'),
  join_paths(sdk_root,'components/libraries/uart/app_uart_fifo.c'),
  join_paths(sdk_root,'components/libraries/util/app_util_platform.c'),
  join_paths(sdk_root,'components/libraries/hardfault/hardfault_implementation.c'),
  join_paths(sdk_root,'components/libraries/util/nrf_assert.c'),
  join_paths(sdk_root,'components/libraries/uart/retarget.c'),
  join_paths(sdk_root,'components/libraries/util/sdk_errors.c'),
  join_paths(sdk_root,'components/boards/boards.c'),
  join_paths(sdk_root,'components/drivers_nrf/clock/nrf_drv_clock.c'),
  join_paths(sdk_root,'components/drivers_nrf/common/nrf_drv_common.c'),
  join_paths(sdk_root,'components/drivers_nrf/gpiote/nrf_drv_gpiote.c'),
  join_paths(sdk_root,'components/drivers_nrf/uart/nrf_drv_uart.c'),
  join_paths(sdk_root,'components/libraries/bsp/bsp.c'),
  join_paths(sdk_root,'components/libraries/bsp/bsp_btn_ble.c'),
  join_paths(sdk_root,'components/libraries/bsp/bsp_nfc.c'),
  join_paths(sdk_root,'external/segger_rtt/RTT_Syscalls_GCC.c'),
  join_paths(sdk_root,'external/segger_rtt/SEGGER_RTT.c'),
  join_paths(sdk_root,'external/segger_rtt/SEGGER_RTT_printf.c'),
  join_paths(sdk_root,'components/ble/common/ble_advdata.c'),
  join_paths(sdk_root,'components/ble/common/ble_conn_params.c'),
  join_paths(sdk_root,'components/ble/ble_db_discovery/ble_db_discovery.c'),
  join_paths(sdk_root,'components/ble/common/ble_srv_common.c'),
  join_paths(sdk_root,'components/toolchain/gcc/gcc_startup_nrf51.S'),
  join_paths(sdk_root,'components/toolchain/system_nrf51.c'),
  join_paths(sdk_root,'components/ble/ble_services/ble_nus_c/ble_nus_c.c'),
  join_paths(sdk_root,'components/softdevice/common/softdevice_handler/softdevice_handler.c'),

  # join_paths(sdk_root,'components/libraries/pwm/app_pwm.c'),
  # join_paths(sdk_root,'components/libraries/scheduler/app_scheduler.c'),
  # join_paths(sdk_root,'components/libraries/pwr_mgmt/nrf_pwr_mgmt.c'),
  # join_paths(sdk_root,'components/libraries/util/sdk_mapped_flags.c'),
  # join_paths(sdk_root,'components/drivers_nrf/nrf_soc_nosd/nrf_nvic.c'),
  # join_paths(sdk_root,'components/drivers_nrf/nrf_soc_nosd/nrf_soc.c'),
  # join_paths(sdk_root,'components/drivers_nrf/timer/nrf_drv_timer.c'),
  # join_paths(sdk_root,'components/drivers_nrf/ppi/nrf_drv_ppi.c'),
  # join_paths(sdk_root,'components/ble/common/ble_conn_state.c'),


  # join_paths(sdk_root,'components/libraries/led_softblink/led_softblink.c'),
  # join_paths(sdk_root,'components/libraries/low_power_pwm/low_power_pwm.c'),
])

# Aqui se definen las carpetas donde estan todos los headers ocupados en el programa 
includes = include_directories([
    '',
    'ble_lib/imu_lib',
    'ble_lib/fes_lib',
    'ble_lib',
    # 'boards',
    # 'servo_lib',
#  #   'custom_ble_services',

    join_paths(sdk_root,''),
    join_paths(sdk_root,'components/drivers_nrf/comp'),
    join_paths(sdk_root,'components/drivers_nrf/twi_master'),
    join_paths(sdk_root,'components/ble/ble_services/ble_ancs_c'),
    join_paths(sdk_root,'components/ble/ble_services/ble_ias_c'),
    join_paths(sdk_root,'components/softdevice/s130/headers'),
    join_paths(sdk_root,'components/libraries/pwm'),
    join_paths(sdk_root,'components/libraries/usbd/class/cdc/acm'),
    join_paths(sdk_root,'components/libraries/usbd/class/hid/generic'),
    join_paths(sdk_root,'components/libraries/usbd/class/msc'),
    join_paths(sdk_root,'components/libraries/usbd/class/hid'),
    join_paths(sdk_root,'components/libraries/log'),
    join_paths(sdk_root,'components/ble/ble_services/ble_gls'),
    join_paths(sdk_root,'components/libraries/fstorage'),
    join_paths(sdk_root,'components/drivers_nrf/i2s'),
    join_paths(sdk_root,'components/libraries/gpiote'),
    join_paths(sdk_root,'components/drivers_nrf/gpiote'),
    join_paths(sdk_root,'components/libraries/fifo'),
    join_paths(sdk_root,'components/boards'),
    join_paths(sdk_root,'components/drivers_nrf/common'),
    join_paths(sdk_root,'components/ble/ble_advertising'),
    join_paths(sdk_root,'components/drivers_nrf/adc'),
    join_paths(sdk_root,'components/softdevice/s130/headers/nrf51'),
    join_paths(sdk_root,'components/ble/ble_services/ble_bas_c'),
    join_paths(sdk_root,'components/ble/ble_services/ble_hrs_c'),
    join_paths(sdk_root,'components/libraries/queue'),
    join_paths(sdk_root,'components/ble/ble_services/ble_rscs_c'),
    join_paths(sdk_root,'components/drivers_nrf/uart'),
    join_paths(sdk_root,'components/ble/common'),
    join_paths(sdk_root,'components/ble/ble_services/ble_lls'),
    join_paths(sdk_root,'components/drivers_nrf/wdt'),
    join_paths(sdk_root,'components/libraries/bsp'),
    join_paths(sdk_root,'components/ble/ble_db_discovery'),
    join_paths(sdk_root,'components/ble/ble_services/ble_bas'),
    join_paths(sdk_root,'components/libraries/experimental_section_vars'),
    join_paths(sdk_root,'components/libraries/usbd/class/cdc'),
    join_paths(sdk_root,'components/drivers_nrf/hal'),
    join_paths(sdk_root,'components/ble/ble_services/ble_nus_c'),
    join_paths(sdk_root,'components/drivers_nrf/rtc'),
    join_paths(sdk_root,'components/ble/ble_services/ble_ias'),
    join_paths(sdk_root,'components/libraries/usbd/class/hid/mouse'),
    join_paths(sdk_root,'components/drivers_nrf/ppi'),
    join_paths(sdk_root,'components/ble/ble_services/ble_dfu'),
    join_paths(sdk_root,'components/drivers_nrf/twis_slave'),
    join_paths(sdk_root,'components'),
    join_paths(sdk_root,'components/libraries/scheduler'),
    join_paths(sdk_root,'components/ble/ble_services/ble_lbs'),
    join_paths(sdk_root,'components/ble/ble_services/ble_hts'),
    join_paths(sdk_root,'components/drivers_nrf/delay'),
    join_paths(sdk_root,'components/libraries/crc16'),
    join_paths(sdk_root,'components/drivers_nrf/timer'),
    join_paths(sdk_root,'components/libraries/util'),
    join_paths(sdk_root,'components/drivers_nrf/pwm'),
    join_paths(sdk_root,'components/libraries/csense_drv'),
    join_paths(sdk_root,'components/libraries/csense'),
    join_paths(sdk_root,'components/drivers_nrf/rng'),
    join_paths(sdk_root,'components/libraries/low_power_pwm'),
    join_paths(sdk_root,'components/libraries/hardfault'),
    join_paths(sdk_root,'components/ble/ble_services/ble_cscs'),
    join_paths(sdk_root,'components/libraries/uart'),
    join_paths(sdk_root,'components/libraries/hci'),
    join_paths(sdk_root,'components/libraries/usbd/class/hid/kbd'),
    join_paths(sdk_root,'components/drivers_nrf/spi_slave'),
    join_paths(sdk_root,'components/drivers_nrf/lpcomp'),
    join_paths(sdk_root,'components/libraries/timer'),
    join_paths(sdk_root,'components/drivers_nrf/power'),
    join_paths(sdk_root,'components/libraries/usbd/config'),
    join_paths(sdk_root,'components/toolchain'),
    join_paths(sdk_root,'components/libraries/led_softblink'),
    join_paths(sdk_root,'components/drivers_nrf/qdec'),
    join_paths(sdk_root,'components/ble/ble_services/ble_cts_c'),
    join_paths(sdk_root,'components/drivers_nrf/spi_master'),
    join_paths(sdk_root,'components/ble/ble_services/ble_nus'),
    join_paths(sdk_root,'components/ble/ble_services/ble_hids'),
    join_paths(sdk_root,'components/drivers_nrf/pdm'),
    join_paths(sdk_root,'components/libraries/crc32'),
    join_paths(sdk_root,'components/libraries/usbd/class/audio'),
    join_paths(sdk_root,'components/ble/peer_manager'),
    join_paths(sdk_root,'components/drivers_nrf/swi'),
    join_paths(sdk_root,'components/ble/ble_services/ble_tps'),
    join_paths(sdk_root,'components/ble/ble_services/ble_dis'),
    join_paths(sdk_root,'components/device'),
    join_paths(sdk_root,'components/ble/nrf_ble_qwr'),
    join_paths(sdk_root,'components/libraries/button'),
    join_paths(sdk_root,'components/libraries/usbd'),
    join_paths(sdk_root,'components/drivers_nrf/saadc'),
    join_paths(sdk_root,'components/ble/ble_services/ble_lbs_c'),
    join_paths(sdk_root,'components/ble/ble_racp'),
    join_paths(sdk_root,'components/toolchain/gcc'),
    join_paths(sdk_root,'components/libraries/fds'),
    join_paths(sdk_root,'components/libraries/twi'),
    join_paths(sdk_root,'components/drivers_nrf/clock'),
    join_paths(sdk_root,'components/ble/ble_services/ble_rscs'),
    join_paths(sdk_root,'components/drivers_nrf/usbd'),
    join_paths(sdk_root,'components/softdevice/common/softdevice_handler'),
    join_paths(sdk_root,'components/ble/ble_services/ble_hrs'),
    join_paths(sdk_root,'components/libraries/log/src'),
    join_paths(sdk_root,'components/toolchain/cmsis/include'),
    
    # join_paths(sdk_root,'components/libraries/pwr_mgmt'),
    # join_paths(sdk_root,'components/ble/ble_services/ble_ans_c'),
    # join_paths(sdk_root,'components/libraries/slip'),
    # join_paths(sdk_root,'components/libraries/mem_manager'),
    # join_paths(sdk_root,'components/drivers_nrf/nrf_soc_nosd'),
])


# Con el siguiente comando se genera el archivo .hex correspondiente al build,"Executable and Linkable File"
# es un formato para archivos ejecutables
elf = executable(
    'minimal.elf',
    sources,
    include_directories : includes,
    link_depends : [linker_script]
)


# El comando para crear el archivo punto hex a partir del elf es
# objcopy -O ihex minimal.elf minimal.hex   /// >> Esto provoca que a partir del archivo
# minimal.elf, se cree un archivo hex con nombre minimal.hex
hexfile = custom_target(
    'hexfile',
    #build_by_default: true,
    output : 'minimal.hex',
    input : elf,
    command : ['objcopy', '-O', 'ihex', '@INPUT@', '@OUTPUT@'],
    
)

#Merge de la apliacion + softdevice
app_and_sdk = custom_target(
    'nrf51_ble_blinky_hex',
    build_by_default: true,
    input: [softdevice, hexfile],
    output: 'nrf51_ble_test_project.hex',
    command: [mergehex, '-m', '@INPUT0@','@INPUT1@','-o','@OUTPUT@']

)

flash_script = run_target('flash',
  command : ['pyocd', 'flash', elf, '-t', 'nrf51']
)

hexflash_script = run_target('hexflash',
  command : ['pyocd', 'flash', app_and_sdk, '-t', 'nrf51']
)

boardflash_script = run_target('boardflash',
  command : ['nrfjprog', '--program', app_and_sdk, '-f', 'nrf51', '--sectorerase', '--reset']
)

gdb_script = run_target('gdb',
command : ['arm-none-eabi-gdb', elf]
)

gdb_server = run_target('gdbserver',
command : ['pyocd', 'gdb', '-t', 'nrf51']
)