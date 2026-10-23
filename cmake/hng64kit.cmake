# Core rules hng64kit
set(HNG64KIT_DIR "${CMAKE_CURRENT_LIST_DIR}/..")
set(HNG64KIT_CRT0 "${HNG64KIT_DIR}/src/crt0.s")
set(HNG64KIT_LDSCRIPT "${HNG64KIT_DIR}/src/hng64.ld")
set(HNG64KIT_LIB_SOURCES
    "${HNG64KIT_DIR}/src/libhng64/bios.c"
    "${HNG64KIT_DIR}/src/libhng64/core.c"
    "${HNG64KIT_DIR}/src/libhng64/iomcu.c"
    "${HNG64KIT_DIR}/src/libhng64/irq.c"
    "${HNG64KIT_DIR}/src/libhng64/irq_entry.s"
    "${HNG64KIT_DIR}/src/libhng64/sound.c"
    "${HNG64KIT_DIR}/src/libhng64/sprite.c"
    "${HNG64KIT_DIR}/src/libhng64/text.c"
    "${HNG64KIT_DIR}/src/libhng64/timer.c"
)

set(HNG64KIT_MKPROGROM "${HNG64_PREFIX}/bin/hng64kit-mkprogrom")
set(HNG64KIT_MKROM "${HNG64_PREFIX}/bin/hng64kit-mkrom")
set(HNG64KIT_MKFONT "${HNG64_PREFIX}/bin/hng64kit-mkfont")
set(HNG64KIT_MKSPRITEROM "${HNG64_PREFIX}/bin/hng64kit-mkspriterom")
set(HNG64KIT_FONT "${HNG64KIT_DIR}/tools/hng64kit-mkfont/font.txt")

function(hng64_add_rom target)
    set(rom_dir "${CMAKE_CURRENT_BINARY_DIR}/roms")
    set(prog_rom "${CMAKE_CURRENT_BINARY_DIR}/${target}.prog.bin")
    set(font_sc "${CMAKE_CURRENT_BINARY_DIR}/font.sc.bin")
    set(sprites_sp "${CMAKE_CURRENT_BINARY_DIR}/sprites.bin")
    set(sprites_h "${CMAKE_CURRENT_BINARY_DIR}/sprites.h")
    set(mkrom_args --program-rom "${prog_rom}" --sc "${font_sc}")
    set(mkrom_deps "${prog_rom}" "${font_sc}" "${HNG64KIT_MKROM}")
    set(pr_roms "${rom_dir}/roadedge/001pr01b.81" "${rom_dir}/roadedge/001pr02b.82")

    target_sources(${target} PRIVATE "${HNG64KIT_CRT0}" ${HNG64KIT_LIB_SOURCES})

    target_include_directories(${target} PRIVATE "${HNG64KIT_DIR}/src")
    set_target_properties(${target} PROPERTIES SUFFIX ".elf" LINK_DEPENDS "${HNG64KIT_LDSCRIPT}")
    target_link_options(${target} PRIVATE -nostdlib -T "${HNG64KIT_LDSCRIPT}")

    add_custom_command(
        OUTPUT "${prog_rom}"
        COMMAND "${HNG64KIT_MKPROGROM}" --elf $<TARGET_FILE:${target}> --name "${target}" --out "${prog_rom}"
        DEPENDS ${target} "${HNG64KIT_MKPROGROM}"
    )

    # Every cart gets a font
    # @TODO: Optional?
    add_custom_command(
        OUTPUT "${font_sc}"
        COMMAND "${HNG64KIT_MKFONT}" --font "${HNG64KIT_FONT}" --out "${font_sc}"
        DEPENDS "${HNG64KIT_FONT}" "${HNG64KIT_MKFONT}"
    )

    # Set HNG64KIT_SPRITES to a list of pngs
    if(HNG64KIT_SPRITES)
        add_custom_command(
            OUTPUT "${sprites_sp}" "${sprites_h}"
            COMMAND "${HNG64KIT_MKSPRITEROM}" --out "${sprites_sp}" --header "${sprites_h}" ${HNG64KIT_SPRITES}
            DEPENDS ${HNG64KIT_SPRITES} "${HNG64KIT_MKSPRITEROM}"
        )

        target_sources(${target} PRIVATE "${sprites_h}")
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

        list(APPEND mkrom_args --sp "${sprites_sp}")
        list(APPEND mkrom_deps "${sprites_sp}")
    endif()

    add_custom_command(
        OUTPUT ${pr_roms}
        COMMAND "${HNG64KIT_MKROM}" ${mkrom_args} --out "${rom_dir}"
        DEPENDS ${mkrom_deps}
    )

    add_custom_target(${target}_rom ALL DEPENDS ${pr_roms})
endfunction()
