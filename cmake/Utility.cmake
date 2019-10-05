macro(IS_PYTHON_DEBUG)
    execute_process(COMMAND python3 -c "import sys; print(hasattr(sys, 'gettotalrefcount'), end='')" OUTPUT_VARIABLE IS_PY_DEBUG)
endmacro()
