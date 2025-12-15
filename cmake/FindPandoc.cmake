if (NOT DEFINED PANDOC_EXEC)
    find_program(Pandoc_EXECUTABLE
        NAMES pandoc pandoc.exe
        HINTS
            # Windows
            ENV "PROGRAMFILES"
            "$ENV{ProgramFiles}/Pandoc"
            "$ENV{ProgramFiles\(x86\)}/Pandoc"
            # Linux
            /usr/bin
            /usr/local/bin
            /snap/bin
            # MacOS
            /opt/local/bin
            /opt/homebrew/bin
    )

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(Pandoc
        REQUIRED_VARS Pandoc_EXECUTABLE
    )

    if(Pandoc_FOUND)
        set(PANDOC_EXEC "${Pandoc_EXECUTABLE}")
    endif()
endif()
