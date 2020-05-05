echo Cleaning solution files . . .

REM Binaries
rmdir "bin" /q /s
rmdir "bin-int" /q /s

REM Solution files/dirs
rmdir "project\.vs" /q /s

rmdir "project\glfw" /q /s
rmdir "project\imgui" /q /s

del "project\Reignite\Reignite.vcxproj" /q
del "project\Reignite\Reignite.vcxproj.filters" /q
del "project\Reignite\Reignite.vcxproj.user" /q

del "project\Render\Render.vcxproj" /q
del "project\Render\Render.vcxproj.filters" /q
del "project\Render\Render.vcxproj.user" /q

del "project\Reignite.sln" /q

REM Spr-v files
del /s /q /f "project\data\shaders\*.spv"

echo Succesfuly cleaned!
PAUSE