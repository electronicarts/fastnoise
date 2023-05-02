@echo off

rem texture sizes
set /A width=128
set /A height=128
set /A depth=32

rem y to make a noise type, blank to not
set "spatial_real=y"
set "spatial_sphere=y"
set "spatial_coshemi=y"
set "temporal_real_exp=y"
set "temporal_real_gauss=y"
set "spatial_sphere_exp=y"
set "spatial_coshemi_exp=y"
set "spatial_sphere_gauss=y"
set "spatial_coshemi_gauss=y"
set "temporalonly_coshemi=y"

rem ============================ SPATIAL REAL ============================
if not defined spatial_real (goto skip_spatial_real)

if not exist "out/real" mkdir "out/real"

FastNoise.exe real Uniform Box 3 Box 1 product %width% %height% 1 out/real/real_uniform_box3x3
FastNoise.exe real Uniform Box 5 Box 1 product %width% %height% 1 out/real/real_uniform_box5x5
FastNoise.exe real Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/real/real_uniform_gauss1_0
FastNoise.exe real Uniform Binomial 2 Box 1 product %width% %height% 1 out/real/real_uniform_binomial3x3
FastNoise.exe real Uniform Binomial 4 Box 1 product %width% %height% 1 out/real/real_uniform_binomial5x5

FastNoise.exe real Tent Box 3 Box 1 product %width% %height% 1 out/real/real_tent_box3x3
FastNoise.exe real Tent Box 5 Box 1 product %width% %height% 1 out/real/real_tent_box5x5
FastNoise.exe real Tent Gauss 1.0 Box 1 product %width% %height% 1 out/real/real_tent_gauss1_0
FastNoise.exe real Tent Binomial 2 Box 1 product %width% %height% 1 out/real/real_tent_binomial3x3
FastNoise.exe real Tent Binomial 4 Box 1 product %width% %height% 1 out/real/real_tent_binomial5x5

:skip_spatial_real
rem ============================ SPATIAL SPHERE ============================
if not defined spatial_sphere (goto skip_spatial_sphere)

if not exist "out/sphere" mkdir "out/sphere"

FastNoise.exe sphere Uniform Box 3 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_box3x3
FastNoise.exe sphere Uniform Box 5 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_box5x5
FastNoise.exe sphere Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_gauss1_0
FastNoise.exe sphere Uniform Binomial 2 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_binomial3x3
FastNoise.exe sphere Uniform Binomial 4 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_binomial5x5

:skip_spatial_sphere
rem ============================ SPATIAL COSINE HEMISPHERE ============================
if not defined spatial_coshemi (goto skip_spatial_coshemi)

if not exist "out/sphere" mkdir "out/sphere"

FastNoise.exe sphere Cosine Box 3 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_box3x3
FastNoise.exe sphere Cosine Box 5 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_box5x5
FastNoise.exe sphere Cosine Gauss 1.0 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_gauss1_0
FastNoise.exe sphere Cosine Binomial 2 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_binomial3x3
FastNoise.exe sphere Cosine Binomial 4 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_binomial5x5

:skip_spatial_coshemi
rem ============================ TEMPORAL REAL EXP ============================
if not defined temporal_real_exp (goto skip_temporal_real_exp)

if not exist "out/real/temporal/exp/" mkdir "out/real/temporal/exp/"

FastNoise.exe real Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_box3x3_exp0101_product -split
FastNoise.exe real Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_box5x5_exp0101_product -split
FastNoise.exe real Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_gauss1_0_exp0101_product -split
FastNoise.exe real Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial3x3_exp0101_product -split
FastNoise.exe real Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial5x5_exp0101_product -split

FastNoise.exe real Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_box3x3_exp0101_separate05 -split
FastNoise.exe real Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_box5x5_exp0101_separate05 -split
FastNoise.exe real Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_gauss1_0_exp0101_separate05 -split
FastNoise.exe real Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial3x3_exp0101_separate05 -split
FastNoise.exe real Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial5x5_exp0101_separate05 -split

:skip_temporal_real_exp
rem ============================ TEMPORAL REAL GAUSS ============================
if not defined temporal_real_gauss (goto skip_temporal_real_gauss)

if not exist "out/real/temporal/gauss/" mkdir "out/real/temporal/gauss/"

FastNoise.exe real Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_box3x3_Gauss10_product -split
FastNoise.exe real Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_box5x5_Gauss10_product -split
FastNoise.exe real Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_gauss1_0_Gauss10_product -split
FastNoise.exe real Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial3x3_Gauss10_product -split
FastNoise.exe real Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial5x5_Gauss10_product -split

FastNoise.exe real Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_box3x3_Gauss10_separate05 -split
FastNoise.exe real Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_box5x5_Gauss10_separate05 -split
FastNoise.exe real Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_gauss1_0_Gauss10_separate05 -split
FastNoise.exe real Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial3x3_Gauss10_separate05 -split
FastNoise.exe real Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial5x5_Gauss10_separate05 -split

:skip_temporal_real_gauss
rem ============================ SPATIAL SPHERE EXP ============================
if not defined spatial_sphere_exp (goto skip_spatial_sphere_exp)

if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_box3x3_exp0101_product -split
FastNoise.exe sphere Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_box5x5_exp0101_product -split
FastNoise.exe sphere Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_gauss1_0_exp0101_product -split
FastNoise.exe sphere Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial3x3_exp0101_product -split
FastNoise.exe sphere Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial5x5_exp0101_product -split

FastNoise.exe sphere Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_box3x3_exp0101_separate05 -split
FastNoise.exe sphere Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_box5x5_exp0101_separate05 -split
FastNoise.exe sphere Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_gauss1_0_exp0101_separate05 -split
FastNoise.exe sphere Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial3x3_exp0101_separate05 -split
FastNoise.exe sphere Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial5x5_exp0101_separate05 -split

:skip_spatial_sphere_exp
rem ============================ SPATIAL COSINE HEMISPHERE EXP ============================
if not defined spatial_coshemi_exp (goto skip_spatial_coshemi_exp)

if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Cosine Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_box3x3_exp0101_product -split
FastNoise.exe sphere Cosine Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_box5x5_exp0101_product -split
FastNoise.exe sphere Cosine Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_gauss1_0_exp0101_product -split
FastNoise.exe sphere Cosine Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial3x3_exp0101_product -split
FastNoise.exe sphere Cosine Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial5x5_exp0101_product -split

FastNoise.exe sphere Cosine Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box3x3_exp0101_separate05 -split
FastNoise.exe sphere Cosine Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box5x5_exp0101_separate05 -split
FastNoise.exe sphere Cosine Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_gauss1_0_exp0101_separate05 -split
FastNoise.exe sphere Cosine Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial3x3_exp0101_separate05 -split
FastNoise.exe sphere Cosine Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial5x5_exp0101_separate05 -split

:skip_spatial_coshemi_exp
rem ============================ SPATIAL SPHERE GAUSS ============================
if not defined spatial_sphere_gauss (goto skip_spatial_sphere_gauss)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"

FastNoise.exe sphere Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_box3x3_Gauss10_product -split
FastNoise.exe sphere Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_box5x5_Gauss10_product -split
FastNoise.exe sphere Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_gauss1_0_Gauss10_product -split
FastNoise.exe sphere Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial3x3_Gauss10_product -split
FastNoise.exe sphere Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial5x5_Gauss10_product -split

FastNoise.exe sphere Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_box3x3_Gauss10_separate05 -split
FastNoise.exe sphere Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_box5x5_Gauss10_separate05 -split
FastNoise.exe sphere Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_gauss1_0_Gauss10_separate05 -split
FastNoise.exe sphere Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial3x3_Gauss10_separate05 -split
FastNoise.exe sphere Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial5x5_Gauss10_separate05 -split

:skip_spatial_sphere_gauss
rem ============================ SPATIAL COSINE HEMISPHERE GAUSS ============================
if not defined spatial_coshemi_gauss (goto skip_spatial_coshemi_gauss)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"

FastNoise.exe sphere Cosine Box 3 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box3x3_Gauss10_product -split
FastNoise.exe sphere Cosine Box 5 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box5x5_Gauss10_product -split
FastNoise.exe sphere Cosine Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_gauss1_0_Gauss10_product -split
FastNoise.exe sphere Cosine Binomial 2 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial3x3_Gauss10_product -split
FastNoise.exe sphere Cosine Binomial 4 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial5x5_Gauss10_product -split

FastNoise.exe sphere Cosine Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box3x3_Gauss10_separate05 -split
FastNoise.exe sphere Cosine Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box5x5_Gauss10_separate05 -split
FastNoise.exe sphere Cosine Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_gauss1_0_Gauss10_separate05 -split
FastNoise.exe sphere Cosine Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial3x3_Gauss10_separate05 -split
FastNoise.exe sphere Cosine Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial5x5_Gauss10_separate05 -split

:skip_spatial_coshemi_gauss
rem ============================ TEMPORAL ONLY COSINE HEMISPHERE ============================
if not defined temporalonly_coshemi (goto skip_temporalonly_coshemi)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"
if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Cosine Box 1 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box1x1_Gauss10_separate05 -split
FastNoise.exe sphere Cosine Box 1 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box1x1_exp0101_separate05 -split

: skip_temporalonly_coshemi