@echo off

rem texture sizes
set /A width=32
set /A height=32

if not exist "out/gauss" mkdir "out/gauss"

FOR /L %%x IN (1, 1, 1000) DO FastNoise.exe real Gauss Gauss 1.0 Box 1 product %width% %height% 1 out/gauss/real_gauss_gauss1_0_%%x -numsteps 5000

rem FastNoise.exe real Gauss Gauss 1.0 Box 1 product %width% %height% 1 out/gauss/real_gauss_gauss1_0_1000 -numsteps 1000
rem FastNoise.exe real Gauss Gauss 1.0 Box 1 product %width% %height% 1 out/gauss/real_gauss_gauss1_0_5000 -numsteps 5000
rem FastNoise.exe real Gauss Gauss 1.0 Box 1 product %width% %height% 1 out/gauss/real_gauss_gauss1_0_10000 -numsteps 10000

rem FastNoise.exe real Gauss Box 3 Box 1 product %width% %height% 1 out/gauss/real_gauss_box3x3
rem FastNoise.exe real Gauss Box 5 Box 1 product %width% %height% 1 out/gauss/real_gauss_box5x5
rem FastNoise.exe real Gauss Gauss 1.0 Box 1 product %width% %height% 1 out/gauss/real_gauss_gauss1_0
rem FastNoise.exe real Gauss Binomial 2 Box 1 product %width% %height% 1 out/gauss/real_gauss_binomial3x3
rem FastNoise.exe real Gauss Binomial 4 Box 1 product %width% %height% 1 out/gauss/real_gauss_binomial5x5