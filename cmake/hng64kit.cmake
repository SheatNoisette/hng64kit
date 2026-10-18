# Core rules hng64kit
set(HNG64KIT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(HNG64KIT_CRT0 "${HNG64KIT_DIR}/src/crt0.s")
set(HNG64KIT_LDSCRIPT "${HNG64KIT_DIR}/src/hng64.ld")

set(HNG64KIT_MKROM "${HNG64_PREFIX}/bin/hng64kit-mkrom")

function(hng64_add_rom target)
    set(rom_dir "${CMAKE_CURRENT_BINARY_DIR}/roms")
    set(pr_roms "${rom_dir}/roadedge/001pr01b.81" "${rom_dir}/roadedge/001pr02b.82")

    target_sources(${target} PRIVATE "${HNG64KIT_CRT0}")
    set_target_properties(${target} PROPERTIES SUFFIX ".elf" LINK_DEPENDS "${HNG64KIT_LDSCRIPT}")
    target_link_options(${target} PRIVATE -nostdlib -T "${HNG64KIT_LDSCRIPT}")

    add_custom_command(
        OUTPUT ${pr_roms}
        COMMAND "${HNG64KIT_MKROM}" $<TARGET_FILE:${target}> "${rom_dir}"
        DEPENDS ${target} "${HNG64KIT_MKROM}"
    )

    add_custom_target(${target}_rom ALL DEPENDS ${pr_roms})
endfunction()
