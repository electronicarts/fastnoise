@echo off

rem Set random seed. the version rem'd out makes it non deterministic.
rem set "seedcmd="
set "seedcmd=-seed 5489"

rem texture sizes
set /A width=128
set /A height=128
set /A depth=32

rem y to make a noise type, blank to not
set "spatial_real="
set "spatial_circle="
set "spatial_vector2="
set "spatial_vector3="
set "spatial_sphere="
set "spatial_coshemi="
set "temporal_real_exp="
set "temporal_real_gauss="
set "temporal_circle_exp="
set "temporal_circle_gauss="
set "temporal_vector2_exp="
set "temporal_vector2_gauss="
set "temporal_vector3_exp="
set "temporal_vector3_gauss="
set "temporal_sphere_exp="
set "temporal_coshemi_exp="
set "temporal_sphere_gauss="
set "temporal_coshemi_gauss="
set "temporalonly_coshemi="

rem ============================ SPATIAL REAL ============================
if not defined spatial_real (goto skip_spatial_real)

if not exist "out/real" mkdir "out/real"

FastNoise.exe real Uniform Box 3 Box 1 product %width% %height% 1 out/real/real_uniform_box3x3 %seedcmd%
FastNoise.exe real Uniform Box 5 Box 1 product %width% %height% 1 out/real/real_uniform_box5x5 %seedcmd%
FastNoise.exe real Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/real/real_uniform_gauss1_0 %seedcmd%
FastNoise.exe real Uniform Binomial 2 Box 1 product %width% %height% 1 out/real/real_uniform_binomial3x3 %seedcmd%
FastNoise.exe real Uniform Binomial 4 Box 1 product %width% %height% 1 out/real/real_uniform_binomial5x5 %seedcmd%

FastNoise.exe real Tent Box 3 Box 1 product %width% %height% 1 out/real/real_tent_box3x3 %seedcmd%
FastNoise.exe real Tent Box 5 Box 1 product %width% %height% 1 out/real/real_tent_box5x5 %seedcmd%
FastNoise.exe real Tent Gauss 1.0 Box 1 product %width% %height% 1 out/real/real_tent_gauss1_0 %seedcmd%
FastNoise.exe real Tent Binomial 2 Box 1 product %width% %height% 1 out/real/real_tent_binomial3x3 %seedcmd%
FastNoise.exe real Tent Binomial 4 Box 1 product %width% %height% 1 out/real/real_tent_binomial5x5 %seedcmd%

:skip_spatial_real
rem ============================ SPATIAL CIRCLE ============================
if not defined spatial_circle (goto skip_spatial_circle)

if not exist "out/real" mkdir "out/real"

FastNoise.exe circle Uniform Box 3 Box 1 product %width% %height% 1 out/real/circle_uniform_box3x3 %seedcmd%
FastNoise.exe circle Uniform Box 5 Box 1 product %width% %height% 1 out/real/circle_uniform_box5x5 %seedcmd%
FastNoise.exe circle Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/real/circle_uniform_gauss1_0 %seedcmd%
FastNoise.exe circle Uniform Binomial 2 Box 1 product %width% %height% 1 out/real/circle_uniform_binomial3x3 %seedcmd%
FastNoise.exe circle Uniform Binomial 4 Box 1 product %width% %height% 1 out/real/circle_uniform_binomial5x5 %seedcmd%

:skip_spatial_circle
rem ============================ SPATIAL VECTOR2 ============================
if not defined spatial_vector2 (goto skip_spatial_vector2)

if not exist "out/vector2" mkdir "out/vector2"

FastNoise.exe vector2 Uniform Box 3 Box 1 product %width% %height% 1 out/vector2/vector2_uniform_box3x3 %seedcmd%
FastNoise.exe vector2 Uniform Box 5 Box 1 product %width% %height% 1 out/vector2/vector2_uniform_box5x5 %seedcmd%
FastNoise.exe vector2 Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/vector2/vector2_uniform_gauss1_0 %seedcmd%
FastNoise.exe vector2 Uniform Binomial 2 Box 1 product %width% %height% 1 out/vector2/vector2_uniform_binomial3x3 %seedcmd%
FastNoise.exe vector2 Uniform Binomial 4 Box 1 product %width% %height% 1 out/vector2/vector2_uniform_binomial5x5 %seedcmd%

:skip_spatial_vector2
rem ============================ SPATIAL VECTOR3 ============================
if not defined spatial_vector3 (goto skip_spatial_vector3)

if not exist "out/vector3" mkdir "out/vector3"

FastNoise.exe vector3 Uniform Box 3 Box 1 product %width% %height% 1 out/vector3/vector3_uniform_box3x3 %seedcmd%
FastNoise.exe vector3 Uniform Box 5 Box 1 product %width% %height% 1 out/vector3/vector3_uniform_box5x5 %seedcmd%
FastNoise.exe vector3 Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/vector3/vector3_uniform_gauss1_0 %seedcmd%
FastNoise.exe vector3 Uniform Binomial 2 Box 1 product %width% %height% 1 out/vector3/vector3_uniform_binomial3x3 %seedcmd%
FastNoise.exe vector3 Uniform Binomial 4 Box 1 product %width% %height% 1 out/vector3/vector3_uniform_binomial5x5 %seedcmd%

:skip_spatial_vector3
rem ============================ SPATIAL SPHERE ============================
if not defined spatial_sphere (goto skip_spatial_sphere)

if not exist "out/sphere" mkdir "out/sphere"

FastNoise.exe sphere Uniform Box 3 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_box3x3 %seedcmd%
FastNoise.exe sphere Uniform Box 5 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_box5x5 %seedcmd%
FastNoise.exe sphere Uniform Gauss 1.0 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_gauss1_0 %seedcmd%
FastNoise.exe sphere Uniform Binomial 2 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_binomial3x3 %seedcmd%
FastNoise.exe sphere Uniform Binomial 4 Box 1 product %width% %height% 1 out/sphere/sphere_uniform_binomial5x5 %seedcmd%

:skip_spatial_sphere
rem ============================ SPATIAL COSINE HEMISPHERE ============================
if not defined spatial_coshemi (goto skip_spatial_coshemi)

if not exist "out/sphere" mkdir "out/sphere"

FastNoise.exe sphere Cosine Box 3 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_box3x3 %seedcmd%
FastNoise.exe sphere Cosine Box 5 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_box5x5 %seedcmd%
FastNoise.exe sphere Cosine Gauss 1.0 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_gauss1_0 %seedcmd%
FastNoise.exe sphere Cosine Binomial 2 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_binomial3x3 %seedcmd%
FastNoise.exe sphere Cosine Binomial 4 Box 1 product %width% %height% 1 out/sphere/sphere_coshemi_binomial5x5 %seedcmd%

:skip_spatial_coshemi
rem ============================ TEMPORAL REAL EXP ============================
if not defined temporal_real_exp (goto skip_temporal_real_exp)

if not exist "out/real/temporal/exp/" mkdir "out/real/temporal/exp/"

FastNoise.exe real Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe real Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe real Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe real Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe real Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe real Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe real Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe real Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe real Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe real Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/real_uniform_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_real_exp
rem ============================ TEMPORAL REAL GAUSS ============================
if not defined temporal_real_gauss (goto skip_temporal_real_gauss)

if not exist "out/real/temporal/gauss/" mkdir "out/real/temporal/gauss/"

FastNoise.exe real Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe real Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe real Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe real Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe real Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe real Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe real Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe real Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe real Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe real Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/real_uniform_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_real_gauss
rem ============================ TEMPORAL CIRCLE EXP ============================
if not defined temporal_circle_exp (goto skip_temporal_circle_exp)

if not exist "out/real/temporal/exp/" mkdir "out/real/temporal/exp/"

FastNoise.exe circle Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/circle_uniform_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe circle Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/circle_uniform_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe circle Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/circle_uniform_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe circle Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/circle_uniform_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe circle Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/real/temporal/exp/circle_uniform_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe circle Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/circle_uniform_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/circle_uniform_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/circle_uniform_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/circle_uniform_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/real/temporal/exp/circle_uniform_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_circle_exp
rem ============================ TEMPORAL CIRCLE GAUSS ============================
if not defined temporal_circle_gauss (goto skip_temporal_circle_gauss)

if not exist "out/real/temporal/gauss/" mkdir "out/real/temporal/gauss/"

FastNoise.exe circle Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/circle_uniform_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe circle Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/circle_uniform_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe circle Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/circle_uniform_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe circle Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/circle_uniform_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe circle Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/real/temporal/gauss/circle_uniform_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe circle Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/circle_uniform_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/circle_uniform_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/circle_uniform_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/circle_uniform_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe circle Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/real/temporal/gauss/circle_uniform_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_circle_gauss
rem ============================ TEMPORAL VECTOR2 EXP ============================
if not defined temporal_vector2_exp (goto skip_temporal_vector2_exp)

if not exist "out/vector2/temporal/exp/" mkdir "out/vector2/temporal/exp/"

FastNoise.exe vector2 Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe vector2 Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe vector2 Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe vector2 Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector2/temporal/exp/vector2_uniform_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_vector2_exp
rem ============================ TEMPORAL VECTOR2 GAUSS ============================
if not defined temporal_vector2_gauss (goto skip_temporal_vector2_gauss)

if not exist "out/vector2/temporal/gauss/" mkdir "out/vector2/temporal/gauss/"

FastNoise.exe vector2 Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe vector2 Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe vector2 Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe vector2 Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector2 Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector2/temporal/gauss/vector2_uniform_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_vector2_gauss
rem ============================ TEMPORAL VECTOR3 EXP ============================
if not defined temporal_vector3_exp (goto skip_temporal_vector3_exp)

if not exist "out/vector3/temporal/exp/" mkdir "out/vector3/temporal/exp/"

FastNoise.exe vector3 Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe vector3 Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe vector3 Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe vector3 Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/vector3/temporal/exp/vector3_uniform_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_vector3_exp
rem ============================ TEMPORAL VECTOR3 GAUSS ============================
if not defined temporal_vector3_gauss (goto skip_temporal_vector3_gauss)

if not exist "out/vector3/temporal/gauss/" mkdir "out/vector3/temporal/gauss/"

FastNoise.exe vector3 Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe vector3 Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe vector3 Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe vector3 Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe vector3 Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/vector3/temporal/gauss/vector3_uniform_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_vector3_gauss
rem ============================ TEMPORAL SPHERE EXP ============================
if not defined temporal_sphere_exp (goto skip_temporal_sphere_exp)

if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Uniform Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe sphere Uniform Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe sphere Uniform Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe sphere Uniform Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_uniform_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_sphere_exp
rem ============================ TEMPORAL COSINE HEMISPHERE EXP ============================
if not defined temporal_coshemi_exp (goto skip_temporal_coshemi_exp)

if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Cosine Box 3 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_box3x3_exp0101_product -split %seedcmd%
FastNoise.exe sphere Cosine Box 5 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_box5x5_exp0101_product -split %seedcmd%
FastNoise.exe sphere Cosine Gauss 1.0 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_gauss1_0_exp0101_product -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 2 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial3x3_exp0101_product -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 4 exponential 0.1 0.1 product %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial5x5_exp0101_product -split %seedcmd%

FastNoise.exe sphere Cosine Box 3 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Box 5 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box5x5_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Gauss 1.0 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_gauss1_0_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 2 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial3x3_exp0101_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 4 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_binomial5x5_exp0101_separate05 -split %seedcmd%

:skip_temporal_coshemi_exp
rem ============================ TEMPORAL SPHERE GAUSS ============================
if not defined temporal_sphere_gauss (goto skip_temporal_sphere_gauss)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"

FastNoise.exe sphere Uniform Box 3 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Uniform Box 5 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Uniform Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 2 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 4 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe sphere Uniform Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Uniform Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_uniform_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_sphere_gauss
rem ============================ TEMPORAL COSINE HEMISPHERE GAUSS ============================
if not defined temporal_coshemi_gauss (goto skip_temporal_coshemi_gauss)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"

FastNoise.exe sphere Cosine Box 3 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box3x3_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Cosine Box 5 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box5x5_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Cosine Gauss 1.0 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_gauss1_0_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 2 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial3x3_Gauss10_product -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 4 gauss 1.0 product %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial5x5_Gauss10_product -split %seedcmd%

FastNoise.exe sphere Cosine Box 3 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Box 5 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box5x5_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Gauss 1.0 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_gauss1_0_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 2 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial3x3_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Binomial 4 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_binomial5x5_Gauss10_separate05 -split %seedcmd%

:skip_temporal_coshemi_gauss
rem ============================ TEMPORAL ONLY COSINE HEMISPHERE ============================
if not defined temporalonly_coshemi (goto skip_temporalonly_coshemi)

if not exist "out/sphere/gauss" mkdir "out/sphere/gauss"
if not exist "out/sphere/exp" mkdir "out/sphere/exp"

FastNoise.exe sphere Cosine Box 1 gauss 1.0 separate 0.5 %width% %height% %depth% out/sphere/gauss/sphere_coshemi_box1x1_Gauss10_separate05 -split %seedcmd%
FastNoise.exe sphere Cosine Box 1 exponential 0.1 0.1 separate 0.5 %width% %height% %depth% out/sphere/exp/sphere_coshemi_box1x1_exp0101_separate05 -split %seedcmd%

: skip_temporalonly_coshemi