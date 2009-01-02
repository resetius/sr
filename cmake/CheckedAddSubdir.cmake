
macro (add_subdirectory_if_exists _subdir)
	if (EXISTS "./${_subdir}")
		add_subdirectory (${_subdir})
	endif (EXISTS "./${_subdir}")
endmacro(add_subdirectory_if_exists)

