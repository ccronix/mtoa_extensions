set MAYA_PATH=C:/Program Files/Autodesk/Maya2022
set MTOA_PATH=C:/Program Files/Autodesk/Arnold/maya2022

cl demo_translator.cpp -LD -MD -I "%MAYA_PATH%/include" -I "%MTOA_PATH%/include/mtoa" -I "%MTOA_PATH%/include/arnold" -link -LIBPATH:"%MAYA_PATH%/lib" -LIBPATH:"%MTOA_PATH%/lib" ai.lib OpenGl32.lib glu32.lib Foundation.lib OpenMaya.lib OpenMayaRender.lib OpenMayaUI.lib OpenMayaAnim.lib OpenMayaFX.lib mtoa_api.lib -out:demo_translator.dll

del *.exp *.lib *.obj
