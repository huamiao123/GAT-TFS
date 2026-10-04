# Original-source MKL/TFS protocol comparison

All paths use default NUMA policy; no explicit interleave/bind/membind. Original source MKL is FP32; TFS/candidates use BF16 inputs and FP32 outputs.

Primary statistic: minimum of five after one warmup. Secondary median and every raw repetition are retained. Prepared two-layer E2E includes ReLU and intermediate conversion, excludes loading/sorting/initial packing.

| Graph | Method | E2E min ms | E2E median ms | vs source MKL | vs source TFS |
|---|---|---:|---:|---:|---:|
| mycielskian19 | source_original_tfs | 2154.040 | 2244.160 | 2.220x | 1.000x |
| mycielskian19 | source_mkl_fp32 | 4781.200 | 4782.680 | 1.000x | 0.451x |
| mycielskian19 | original_nozero | 2119.656 | 2201.901 | 2.256x | 1.016x |
| mycielskian19 | shared_b2_fast_nozero | 2570.182 | 2631.957 | 1.860x | 0.838x |
| mycielskian19 | shared_b2_accurate_nozero | 3360.469 | 3582.308 | 1.423x | 0.641x |
| mycielskian19 | shared_b4_fast_nozero | 1633.512 | 1755.643 | 2.927x | 1.319x |
| mycielskian19 | shared_b4_accurate_nozero | 2139.674 | 2181.817 | 2.235x | 1.007x |
| mycielskian19 | shared_b8_fast_nozero | 1352.749 | 1368.302 | 3.534x | 1.592x |
| mycielskian19 | shared_b8_accurate_nozero | 1485.979 | 1531.302 | 3.218x | 1.450x |
| mycielskian19 | shared_b16_fast_nozero | 1196.852 | 1215.416 | 3.995x | 1.800x |
| mycielskian19 | shared_b16_accurate_nozero | 1243.416 | 1253.620 | 3.845x | 1.732x |
| mycielskian19 | shared_b32_fast_nozero | 1128.891 | 1161.219 | 4.235x | 1.908x |
| mycielskian19 | shared_b32_accurate_nozero | 1155.431 | 1169.995 | 4.138x | 1.864x |
| mycielskian19 | shared_b64_fast_nozero | 1094.980 | 1106.625 | 4.366x | 1.967x |
| mycielskian19 | shared_b64_accurate_nozero | 1112.184 | 1151.814 | 4.299x | 1.937x |
| mycielskian19 | shared_bfull_fast_nozero | 2436.928 | 2479.818 | 1.962x | 0.884x |
| mycielskian19 | shared_bfull_accurate_nozero | 2432.288 | 2494.789 | 1.966x | 0.886x |
| reddit | source_original_tfs | 307.650 | 320.570 | 1.427x | 1.000x |
| reddit | source_mkl_fp32 | 438.970 | 439.640 | 1.000x | 0.701x |
| reddit | original_nozero | 307.431 | 318.822 | 1.428x | 1.001x |
| reddit | shared_b2_fast_nozero | 340.474 | 345.262 | 1.289x | 0.904x |
| reddit | shared_b2_accurate_nozero | 456.456 | 460.605 | 0.962x | 0.674x |
| reddit | shared_b4_fast_nozero | 230.358 | 235.817 | 1.906x | 1.336x |
| reddit | shared_b4_accurate_nozero | 270.226 | 276.276 | 1.624x | 1.138x |
| reddit | shared_b8_fast_nozero | 179.685 | 180.758 | 2.443x | 1.712x |
| reddit | shared_b8_accurate_nozero | 198.331 | 204.037 | 2.213x | 1.551x |
| reddit | shared_b16_fast_nozero | 150.415 | 153.175 | 2.918x | 2.045x |
| reddit | shared_b16_accurate_nozero | 156.642 | 163.039 | 2.802x | 1.964x |
| reddit | shared_b32_fast_nozero | 137.500 | 140.713 | 3.193x | 2.237x |
| reddit | shared_b32_accurate_nozero | 137.632 | 143.132 | 3.189x | 2.235x |
| reddit | shared_b64_fast_nozero | 130.304 | 132.514 | 3.369x | 2.361x |
| reddit | shared_b64_accurate_nozero | 132.108 | 135.011 | 3.323x | 2.329x |
| reddit | shared_bfull_fast_nozero | 136.334 | 142.161 | 3.220x | 2.257x |
| reddit | shared_bfull_accurate_nozero | 138.025 | 142.760 | 3.180x | 2.229x |
| hollywood-2009 | source_original_tfs | 362.210 | 362.840 | 1.064x | 1.000x |
| hollywood-2009 | source_mkl_fp32 | 385.420 | 385.670 | 1.000x | 0.940x |
| hollywood-2009 | original_nozero | 334.154 | 339.735 | 1.153x | 1.084x |
| hollywood-2009 | shared_b2_fast_nozero | 380.930 | 383.386 | 1.012x | 0.951x |
| hollywood-2009 | shared_b2_accurate_nozero | 473.973 | 477.752 | 0.813x | 0.764x |
| hollywood-2009 | shared_b4_fast_nozero | 292.692 | 293.600 | 1.317x | 1.238x |
| hollywood-2009 | shared_b4_accurate_nozero | 332.801 | 337.606 | 1.158x | 1.088x |
| hollywood-2009 | shared_b8_fast_nozero | 244.999 | 247.758 | 1.573x | 1.478x |
| hollywood-2009 | shared_b8_accurate_nozero | 265.726 | 267.154 | 1.450x | 1.363x |
| hollywood-2009 | shared_b16_fast_nozero | 229.054 | 230.500 | 1.683x | 1.581x |
| hollywood-2009 | shared_b16_accurate_nozero | 234.806 | 237.887 | 1.641x | 1.543x |
| hollywood-2009 | shared_b32_fast_nozero | 228.263 | 230.980 | 1.688x | 1.587x |
| hollywood-2009 | shared_b32_accurate_nozero | 229.394 | 232.460 | 1.680x | 1.579x |
| hollywood-2009 | shared_b64_fast_nozero | 224.257 | 229.350 | 1.719x | 1.615x |
| hollywood-2009 | shared_b64_accurate_nozero | 224.660 | 225.638 | 1.716x | 1.612x |
| hollywood-2009 | shared_bfull_fast_nozero | 227.336 | 231.262 | 1.695x | 1.593x |
| hollywood-2009 | shared_bfull_accurate_nozero | 225.525 | 228.154 | 1.709x | 1.606x |
| ogbn-products | source_original_tfs | 466.090 | 471.360 | 2.885x | 1.000x |
| ogbn-products | source_mkl_fp32 | 1344.750 | 1345.420 | 1.000x | 0.347x |
| ogbn-products | original_nozero | 392.670 | 402.105 | 3.425x | 1.187x |
| ogbn-products | shared_b2_fast_nozero | 596.819 | 603.305 | 2.253x | 0.781x |
| ogbn-products | shared_b2_accurate_nozero | 703.965 | 717.039 | 1.910x | 0.662x |
| ogbn-products | shared_b4_fast_nozero | 487.102 | 491.268 | 2.761x | 0.957x |
| ogbn-products | shared_b4_accurate_nozero | 539.472 | 545.671 | 2.493x | 0.864x |
| ogbn-products | shared_b8_fast_nozero | 425.195 | 430.741 | 3.163x | 1.096x |
| ogbn-products | shared_b8_accurate_nozero | 451.028 | 453.760 | 2.982x | 1.033x |
| ogbn-products | shared_b16_fast_nozero | 390.168 | 393.532 | 3.447x | 1.195x |
| ogbn-products | shared_b16_accurate_nozero | 405.991 | 412.096 | 3.312x | 1.148x |
| ogbn-products | shared_b32_fast_nozero | 358.210 | 359.630 | 3.754x | 1.301x |
| ogbn-products | shared_b32_accurate_nozero | 380.196 | 382.767 | 3.537x | 1.226x |
| ogbn-products | shared_b64_fast_nozero | 351.198 | 353.556 | 3.829x | 1.327x |
| ogbn-products | shared_b64_accurate_nozero | 363.497 | 366.786 | 3.699x | 1.282x |
| ogbn-products | shared_bfull_fast_nozero | 346.496 | 347.508 | 3.881x | 1.345x |
| ogbn-products | shared_bfull_accurate_nozero | 355.184 | 359.135 | 3.786x | 1.312x |
| indochina-2004 | source_original_tfs | 830.930 | 833.020 | 0.833x | 1.000x |
| indochina-2004 | source_mkl_fp32 | 692.310 | 695.680 | 1.000x | 1.200x |
| indochina-2004 | original_nozero | 590.994 | 592.029 | 1.171x | 1.406x |
| indochina-2004 | shared_b2_fast_nozero | 787.376 | 789.346 | 0.879x | 1.055x |
| indochina-2004 | shared_b2_accurate_nozero | 939.306 | 940.807 | 0.737x | 0.885x |
| indochina-2004 | shared_b4_fast_nozero | 624.213 | 626.754 | 1.109x | 1.331x |
| indochina-2004 | shared_b4_accurate_nozero | 708.622 | 708.756 | 0.977x | 1.173x |
| indochina-2004 | shared_b8_fast_nozero | 544.313 | 546.255 | 1.272x | 1.527x |
| indochina-2004 | shared_b8_accurate_nozero | 585.944 | 588.308 | 1.182x | 1.418x |
| indochina-2004 | shared_b16_fast_nozero | 498.861 | 501.557 | 1.388x | 1.666x |
| indochina-2004 | shared_b16_accurate_nozero | 522.586 | 524.132 | 1.325x | 1.590x |
| indochina-2004 | shared_b32_fast_nozero | 474.186 | 474.783 | 1.460x | 1.752x |
| indochina-2004 | shared_b32_accurate_nozero | 490.586 | 491.905 | 1.411x | 1.694x |
| indochina-2004 | shared_b64_fast_nozero | 461.763 | 463.659 | 1.499x | 1.799x |
| indochina-2004 | shared_b64_accurate_nozero | 475.711 | 476.770 | 1.455x | 1.747x |
| indochina-2004 | shared_bfull_fast_nozero | 456.482 | 457.247 | 1.517x | 1.820x |
| indochina-2004 | shared_bfull_accurate_nozero | 470.035 | 470.571 | 1.473x | 1.768x |
| soc-Pokec | source_original_tfs | 193.070 | 195.540 | 1.710x | 1.000x |
| soc-Pokec | source_mkl_fp32 | 330.130 | 331.380 | 1.000x | 0.585x |
| soc-Pokec | original_nozero | 141.922 | 142.486 | 2.326x | 1.360x |
| soc-Pokec | shared_b2_fast_nozero | 201.066 | 205.097 | 1.642x | 0.960x |
| soc-Pokec | shared_b2_accurate_nozero | 228.215 | 232.832 | 1.447x | 0.846x |
| soc-Pokec | shared_b4_fast_nozero | 174.065 | 177.875 | 1.897x | 1.109x |
| soc-Pokec | shared_b4_accurate_nozero | 187.766 | 188.044 | 1.758x | 1.028x |
| soc-Pokec | shared_b8_fast_nozero | 161.567 | 163.578 | 2.043x | 1.195x |
| soc-Pokec | shared_b8_accurate_nozero | 165.215 | 170.504 | 1.998x | 1.169x |
| soc-Pokec | shared_b16_fast_nozero | 155.061 | 155.549 | 2.129x | 1.245x |
| soc-Pokec | shared_b16_accurate_nozero | 158.877 | 161.513 | 2.078x | 1.215x |
| soc-Pokec | shared_b32_fast_nozero | 146.313 | 147.382 | 2.256x | 1.320x |
| soc-Pokec | shared_b32_accurate_nozero | 153.953 | 154.598 | 2.144x | 1.254x |
| soc-Pokec | shared_b64_fast_nozero | 147.065 | 147.203 | 2.245x | 1.313x |
| soc-Pokec | shared_b64_accurate_nozero | 149.405 | 150.164 | 2.210x | 1.292x |
| soc-Pokec | shared_bfull_fast_nozero | 146.108 | 147.224 | 2.259x | 1.321x |
| soc-Pokec | shared_bfull_accurate_nozero | 148.369 | 149.261 | 2.225x | 1.301x |
| com-LiveJournal | source_original_tfs | 499.310 | 503.820 | 1.289x | 1.000x |
| com-LiveJournal | source_mkl_fp32 | 643.470 | 643.740 | 1.000x | 0.776x |
| com-LiveJournal | original_nozero | 367.974 | 374.675 | 1.749x | 1.357x |
| com-LiveJournal | shared_b2_fast_nozero | 504.957 | 510.612 | 1.274x | 0.989x |
| com-LiveJournal | shared_b2_accurate_nozero | 562.161 | 566.906 | 1.145x | 0.888x |
| com-LiveJournal | shared_b4_fast_nozero | 448.806 | 454.718 | 1.434x | 1.113x |
| com-LiveJournal | shared_b4_accurate_nozero | 479.206 | 480.499 | 1.343x | 1.042x |
| com-LiveJournal | shared_b8_fast_nozero | 421.529 | 426.581 | 1.527x | 1.185x |
| com-LiveJournal | shared_b8_accurate_nozero | 430.247 | 434.593 | 1.496x | 1.161x |
| com-LiveJournal | shared_b16_fast_nozero | 418.213 | 418.866 | 1.539x | 1.194x |
| com-LiveJournal | shared_b16_accurate_nozero | 425.528 | 428.297 | 1.512x | 1.173x |
| com-LiveJournal | shared_b32_fast_nozero | 408.694 | 411.330 | 1.574x | 1.222x |
| com-LiveJournal | shared_b32_accurate_nozero | 416.175 | 419.421 | 1.546x | 1.200x |
| com-LiveJournal | shared_b64_fast_nozero | 408.109 | 409.602 | 1.577x | 1.223x |
| com-LiveJournal | shared_b64_accurate_nozero | 411.290 | 413.717 | 1.565x | 1.214x |
| com-LiveJournal | shared_bfull_fast_nozero | 403.996 | 407.438 | 1.593x | 1.236x |
| com-LiveJournal | shared_bfull_accurate_nozero | 408.624 | 411.183 | 1.575x | 1.222x |
| soc-LiveJournal1 | source_original_tfs | 594.270 | 598.310 | 1.144x | 1.000x |
| soc-LiveJournal1 | source_mkl_fp32 | 679.700 | 680.340 | 1.000x | 0.874x |
| soc-LiveJournal1 | original_nozero | 428.099 | 439.515 | 1.588x | 1.388x |
| soc-LiveJournal1 | shared_b2_fast_nozero | 583.977 | 589.632 | 1.164x | 1.018x |
| soc-LiveJournal1 | shared_b2_accurate_nozero | 636.913 | 659.896 | 1.067x | 0.933x |
| soc-LiveJournal1 | shared_b4_fast_nozero | 533.720 | 541.045 | 1.274x | 1.113x |
| soc-LiveJournal1 | shared_b4_accurate_nozero | 562.876 | 568.670 | 1.208x | 1.056x |
| soc-LiveJournal1 | shared_b8_fast_nozero | 504.543 | 506.162 | 1.347x | 1.178x |
| soc-LiveJournal1 | shared_b8_accurate_nozero | 523.301 | 525.993 | 1.299x | 1.136x |
| soc-LiveJournal1 | shared_b16_fast_nozero | 507.841 | 510.758 | 1.338x | 1.170x |
| soc-LiveJournal1 | shared_b16_accurate_nozero | 522.713 | 523.435 | 1.300x | 1.137x |
| soc-LiveJournal1 | shared_b32_fast_nozero | 497.229 | 499.868 | 1.367x | 1.195x |
| soc-LiveJournal1 | shared_b32_accurate_nozero | 506.143 | 511.550 | 1.343x | 1.174x |
| soc-LiveJournal1 | shared_b64_fast_nozero | 492.390 | 493.683 | 1.380x | 1.207x |
| soc-LiveJournal1 | shared_b64_accurate_nozero | 500.945 | 502.797 | 1.357x | 1.186x |
| soc-LiveJournal1 | shared_bfull_fast_nozero | 492.593 | 493.290 | 1.380x | 1.206x |
| soc-LiveJournal1 | shared_bfull_accurate_nozero | 498.134 | 499.778 | 1.364x | 1.193x |
| as-Skitter | source_original_tfs | 199.670 | 203.210 | 0.815x | 1.000x |
| as-Skitter | source_mkl_fp32 | 162.800 | 162.940 | 1.000x | 1.226x |
| as-Skitter | original_nozero | 144.385 | 149.594 | 1.128x | 1.383x |
| as-Skitter | shared_b2_fast_nozero | 198.413 | 202.224 | 0.821x | 1.006x |
| as-Skitter | shared_b2_accurate_nozero | 235.827 | 241.080 | 0.690x | 0.847x |
| as-Skitter | shared_b4_fast_nozero | 164.599 | 167.684 | 0.989x | 1.213x |
| as-Skitter | shared_b4_accurate_nozero | 181.503 | 184.062 | 0.897x | 1.100x |
| as-Skitter | shared_b8_fast_nozero | 149.017 | 151.093 | 1.092x | 1.340x |
| as-Skitter | shared_b8_accurate_nozero | 156.628 | 158.072 | 1.039x | 1.275x |
| as-Skitter | shared_b16_fast_nozero | 138.464 | 139.396 | 1.176x | 1.442x |
| as-Skitter | shared_b16_accurate_nozero | 141.417 | 144.095 | 1.151x | 1.412x |
| as-Skitter | shared_b32_fast_nozero | 131.149 | 133.895 | 1.241x | 1.522x |
| as-Skitter | shared_b32_accurate_nozero | 137.396 | 138.337 | 1.185x | 1.453x |
| as-Skitter | shared_b64_fast_nozero | 131.482 | 133.207 | 1.238x | 1.519x |
| as-Skitter | shared_b64_accurate_nozero | 132.940 | 133.309 | 1.225x | 1.502x |
| as-Skitter | shared_bfull_fast_nozero | 131.713 | 132.338 | 1.236x | 1.516x |
| as-Skitter | shared_bfull_accurate_nozero | 130.807 | 133.175 | 1.245x | 1.526x |
| email-Enron | source_original_tfs | 3.180 | 3.250 | 0.447x | 1.000x |
| email-Enron | source_mkl_fp32 | 1.420 | 1.440 | 1.000x | 2.239x |
| email-Enron | original_nozero | 2.303 | 2.311 | 0.617x | 1.381x |
| email-Enron | shared_b2_fast_nozero | 3.215 | 3.266 | 0.442x | 0.989x |
| email-Enron | shared_b2_accurate_nozero | 4.924 | 4.941 | 0.288x | 0.646x |
| email-Enron | shared_b4_fast_nozero | 2.275 | 2.320 | 0.624x | 1.398x |
| email-Enron | shared_b4_accurate_nozero | 3.000 | 3.039 | 0.473x | 1.060x |
| email-Enron | shared_b8_fast_nozero | 1.756 | 1.770 | 0.809x | 1.811x |
| email-Enron | shared_b8_accurate_nozero | 2.101 | 2.121 | 0.676x | 1.513x |
| email-Enron | shared_b16_fast_nozero | 1.442 | 1.480 | 0.985x | 2.205x |
| email-Enron | shared_b16_accurate_nozero | 1.620 | 1.722 | 0.877x | 1.963x |
| email-Enron | shared_b32_fast_nozero | 1.299 | 1.344 | 1.093x | 2.448x |
| email-Enron | shared_b32_accurate_nozero | 1.437 | 1.454 | 0.988x | 2.213x |
| email-Enron | shared_b64_fast_nozero | 1.242 | 1.290 | 1.143x | 2.561x |
| email-Enron | shared_b64_accurate_nozero | 1.348 | 1.372 | 1.053x | 2.359x |
| email-Enron | shared_bfull_fast_nozero | 1.200 | 1.237 | 1.183x | 2.650x |
| email-Enron | shared_bfull_accurate_nozero | 1.262 | 1.297 | 1.125x | 2.520x |
| amazon0601 | source_original_tfs | 38.630 | 38.760 | 0.660x | 1.000x |
| amazon0601 | source_mkl_fp32 | 25.500 | 25.550 | 1.000x | 1.515x |
| amazon0601 | original_nozero | 25.397 | 25.614 | 1.004x | 1.521x |
| amazon0601 | shared_b2_fast_nozero | 29.213 | 29.498 | 0.873x | 1.322x |
| amazon0601 | shared_b2_accurate_nozero | 31.804 | 32.151 | 0.802x | 1.215x |
| amazon0601 | shared_b4_fast_nozero | 27.325 | 27.397 | 0.933x | 1.414x |
| amazon0601 | shared_b4_accurate_nozero | 28.852 | 29.089 | 0.884x | 1.339x |
| amazon0601 | shared_b8_fast_nozero | 25.815 | 26.176 | 0.988x | 1.496x |
| amazon0601 | shared_b8_accurate_nozero | 26.995 | 27.181 | 0.945x | 1.431x |
| amazon0601 | shared_b16_fast_nozero | 25.123 | 25.511 | 1.015x | 1.538x |
| amazon0601 | shared_b16_accurate_nozero | 25.999 | 26.366 | 0.981x | 1.486x |
| amazon0601 | shared_b32_fast_nozero | 25.361 | 25.538 | 1.005x | 1.523x |
| amazon0601 | shared_b32_accurate_nozero | 25.875 | 26.079 | 0.986x | 1.493x |
| amazon0601 | shared_b64_fast_nozero | 25.422 | 25.745 | 1.003x | 1.520x |
| amazon0601 | shared_b64_accurate_nozero | 26.325 | 26.380 | 0.969x | 1.467x |
| amazon0601 | shared_bfull_fast_nozero | 25.151 | 25.324 | 1.014x | 1.536x |
| amazon0601 | shared_bfull_accurate_nozero | 26.288 | 26.459 | 0.970x | 1.469x |
| web-Google | source_original_tfs | 75.780 | 76.520 | 0.613x | 1.000x |
| web-Google | source_mkl_fp32 | 46.470 | 46.960 | 1.000x | 1.631x |
| web-Google | original_nozero | 46.849 | 47.011 | 0.992x | 1.618x |
| web-Google | shared_b2_fast_nozero | 57.196 | 58.068 | 0.812x | 1.325x |
| web-Google | shared_b2_accurate_nozero | 60.538 | 61.395 | 0.768x | 1.252x |
| web-Google | shared_b4_fast_nozero | 54.049 | 54.553 | 0.860x | 1.402x |
| web-Google | shared_b4_accurate_nozero | 55.597 | 55.800 | 0.836x | 1.363x |
| web-Google | shared_b8_fast_nozero | 52.551 | 52.966 | 0.884x | 1.442x |
| web-Google | shared_b8_accurate_nozero | 53.176 | 53.363 | 0.874x | 1.425x |
| web-Google | shared_b16_fast_nozero | 51.292 | 51.584 | 0.906x | 1.477x |
| web-Google | shared_b16_accurate_nozero | 52.147 | 52.370 | 0.891x | 1.453x |
| web-Google | shared_b32_fast_nozero | 50.925 | 51.160 | 0.913x | 1.488x |
| web-Google | shared_b32_accurate_nozero | 52.002 | 52.145 | 0.894x | 1.457x |
| web-Google | shared_b64_fast_nozero | 50.789 | 50.919 | 0.915x | 1.492x |
| web-Google | shared_b64_accurate_nozero | 51.466 | 51.850 | 0.903x | 1.472x |
| web-Google | shared_bfull_fast_nozero | 50.655 | 50.929 | 0.917x | 1.496x |
| web-Google | shared_bfull_accurate_nozero | 51.745 | 51.904 | 0.898x | 1.464x |
| com-Youtube | source_original_tfs | 131.290 | 131.690 | 0.408x | 1.000x |
| com-Youtube | source_mkl_fp32 | 53.600 | 53.910 | 1.000x | 2.449x |
| com-Youtube | original_nozero | 94.367 | 95.345 | 0.568x | 1.391x |
| com-Youtube | shared_b2_fast_nozero | 118.037 | 120.085 | 0.454x | 1.112x |
| com-Youtube | shared_b2_accurate_nozero | 139.994 | 142.595 | 0.383x | 0.938x |
| com-Youtube | shared_b4_fast_nozero | 100.139 | 100.793 | 0.535x | 1.311x |
| com-Youtube | shared_b4_accurate_nozero | 113.412 | 113.718 | 0.473x | 1.158x |
| com-Youtube | shared_b8_fast_nozero | 91.379 | 91.884 | 0.587x | 1.437x |
| com-Youtube | shared_b8_accurate_nozero | 97.818 | 98.583 | 0.548x | 1.342x |
| com-Youtube | shared_b16_fast_nozero | 86.626 | 87.445 | 0.619x | 1.516x |
| com-Youtube | shared_b16_accurate_nozero | 92.200 | 92.673 | 0.581x | 1.424x |
| com-Youtube | shared_b32_fast_nozero | 83.303 | 84.082 | 0.643x | 1.576x |
| com-Youtube | shared_b32_accurate_nozero | 88.374 | 89.043 | 0.607x | 1.486x |
| com-Youtube | shared_b64_fast_nozero | 82.522 | 83.070 | 0.650x | 1.591x |
| com-Youtube | shared_b64_accurate_nozero | 86.087 | 87.632 | 0.623x | 1.525x |
| com-Youtube | shared_bfull_fast_nozero | 81.085 | 81.703 | 0.661x | 1.619x |
| com-Youtube | shared_bfull_accurate_nozero | 85.086 | 85.665 | 0.630x | 1.543x |
| cit-Patents | source_original_tfs | 312.010 | 313.170 | 0.866x | 1.000x |
| cit-Patents | source_mkl_fp32 | 270.190 | 271.090 | 1.000x | 1.155x |
| cit-Patents | original_nozero | 191.135 | 192.834 | 1.414x | 1.632x |
| cit-Patents | shared_b2_fast_nozero | 231.955 | 233.484 | 1.165x | 1.345x |
| cit-Patents | shared_b2_accurate_nozero | 247.414 | 247.624 | 1.092x | 1.261x |
| cit-Patents | shared_b4_fast_nozero | 217.529 | 217.862 | 1.242x | 1.434x |
| cit-Patents | shared_b4_accurate_nozero | 225.997 | 226.688 | 1.196x | 1.381x |
| cit-Patents | shared_b8_fast_nozero | 207.488 | 208.629 | 1.302x | 1.504x |
| cit-Patents | shared_b8_accurate_nozero | 215.804 | 216.133 | 1.252x | 1.446x |
| cit-Patents | shared_b16_fast_nozero | 202.897 | 203.561 | 1.332x | 1.538x |
| cit-Patents | shared_b16_accurate_nozero | 210.406 | 211.458 | 1.284x | 1.483x |
| cit-Patents | shared_b32_fast_nozero | 200.753 | 201.644 | 1.346x | 1.554x |
| cit-Patents | shared_b32_accurate_nozero | 208.183 | 209.922 | 1.298x | 1.499x |
| cit-Patents | shared_b64_fast_nozero | 201.003 | 201.524 | 1.344x | 1.552x |
| cit-Patents | shared_b64_accurate_nozero | 208.903 | 210.011 | 1.293x | 1.494x |
| cit-Patents | shared_bfull_fast_nozero | 201.534 | 201.865 | 1.341x | 1.548x |
| cit-Patents | shared_bfull_accurate_nozero | 208.214 | 209.426 | 1.298x | 1.499x |
| roadNet-CA | source_original_tfs | 167.270 | 167.630 | 0.387x | 1.000x |
| roadNet-CA | source_mkl_fp32 | 64.730 | 64.950 | 1.000x | 2.584x |
| roadNet-CA | original_nozero | 103.136 | 103.968 | 0.628x | 1.622x |
| roadNet-CA | shared_b2_fast_nozero | 117.250 | 117.471 | 0.552x | 1.427x |
| roadNet-CA | shared_b2_accurate_nozero | 122.047 | 122.646 | 0.530x | 1.371x |
| roadNet-CA | shared_b4_fast_nozero | 110.145 | 110.816 | 0.588x | 1.519x |
| roadNet-CA | shared_b4_accurate_nozero | 112.833 | 113.125 | 0.574x | 1.482x |
| roadNet-CA | shared_b8_fast_nozero | 110.343 | 110.596 | 0.587x | 1.516x |
| roadNet-CA | shared_b8_accurate_nozero | 112.797 | 113.579 | 0.574x | 1.483x |
| roadNet-CA | shared_b16_fast_nozero | 109.755 | 111.056 | 0.590x | 1.524x |
| roadNet-CA | shared_b16_accurate_nozero | 113.402 | 114.048 | 0.571x | 1.475x |
| roadNet-CA | shared_b32_fast_nozero | 110.638 | 110.972 | 0.585x | 1.512x |
| roadNet-CA | shared_b32_accurate_nozero | 112.535 | 113.448 | 0.575x | 1.486x |
| roadNet-CA | shared_b64_fast_nozero | 109.980 | 110.343 | 0.589x | 1.521x |
| roadNet-CA | shared_b64_accurate_nozero | 112.675 | 113.071 | 0.574x | 1.485x |
| roadNet-CA | shared_bfull_fast_nozero | 110.244 | 110.819 | 0.587x | 1.517x |
| roadNet-CA | shared_bfull_accurate_nozero | 112.348 | 113.373 | 0.576x | 1.489x |
| wiki-Talk | source_original_tfs | 291.180 | 294.710 | 0.311x | 1.000x |
| wiki-Talk | source_mkl_fp32 | 90.580 | 90.650 | 1.000x | 3.215x |
| wiki-Talk | original_nozero | 213.985 | 215.356 | 0.423x | 1.361x |
| wiki-Talk | shared_b2_fast_nozero | 267.157 | 269.343 | 0.339x | 1.090x |
| wiki-Talk | shared_b2_accurate_nozero | 324.579 | 330.701 | 0.279x | 0.897x |
| wiki-Talk | shared_b4_fast_nozero | 212.770 | 213.372 | 0.426x | 1.369x |
| wiki-Talk | shared_b4_accurate_nozero | 234.534 | 241.170 | 0.386x | 1.242x |
| wiki-Talk | shared_b8_fast_nozero | 177.372 | 183.378 | 0.511x | 1.642x |
| wiki-Talk | shared_b8_accurate_nozero | 196.346 | 197.656 | 0.461x | 1.483x |
| wiki-Talk | shared_b16_fast_nozero | 163.563 | 166.585 | 0.554x | 1.780x |
| wiki-Talk | shared_b16_accurate_nozero | 170.831 | 171.274 | 0.530x | 1.704x |
| wiki-Talk | shared_b32_fast_nozero | 158.615 | 159.480 | 0.571x | 1.836x |
| wiki-Talk | shared_b32_accurate_nozero | 156.461 | 162.825 | 0.579x | 1.861x |
| wiki-Talk | shared_b64_fast_nozero | 155.275 | 158.121 | 0.583x | 1.875x |
| wiki-Talk | shared_b64_accurate_nozero | 154.644 | 158.527 | 0.586x | 1.883x |
| wiki-Talk | shared_bfull_fast_nozero | 151.173 | 154.662 | 0.599x | 1.926x |
| wiki-Talk | shared_bfull_accurate_nozero | 151.884 | 153.425 | 0.596x | 1.917x |

## Status and exclusions

- mycielskian19: PASS
- reddit: PASS
- hollywood-2009: PASS
- kron_g500-logn21: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- com-Friendster: SKIPPED — Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol
- ogbn-products: PASS
- indochina-2004: PASS
- cage15: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- soc-Pokec: PASS
- com-LiveJournal: PASS
- rgg_n_2_24_s0: SKIPPED — Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol
- soc-LiveJournal1: PASS
- as-Skitter: PASS
- email-Enron: PASS
- FullChip: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- amazon0601: PASS
- scircuit: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- web-Google: PASS
- com-Youtube: PASS
- cit-Patents: PASS
- sx-stackoverflow: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- rajat31: SKIPPED — Previously verified nonunit CSR values; original TFS ignores values while MKL uses them
- roadNet-CA: PASS
- road_usa: SKIPPED — Original LP64 MKL range limit; ILP64 changes are outside source-faithful protocol
- wiki-Talk: PASS

Historical results with explicit NUMA interleave are a different protocol and must not be combined with this table.
Baseline E2E values retain the original 0.01 ms stdout rounding. Stage/profile experiments are separate from primary timing; sampled thread time is not wall-time percentage.
Random H/W exercise the source inference operator; trained-model accuracy and full application end-to-end are not claimed.
