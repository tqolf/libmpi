# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

|              description              | average time | average time(reference) | coefficient of variation | perfermance ratio |
| :-: | :-: | :-: | :-: | :-: |
|       from-string(mpi vs ossl)        |   2131.14    |         24367.3         |        0.0217299         |      <span style="color:#8000;">**11.4339**</span>      |
|        to-string(mpi vs ossl)         |    574.51    |         2219.52         |        0.0418995         |      <span style="color:#8000;">**3.86332**</span>      |
|       from-octets(mpi vs ossl)        |   250.024    |         553.09          |        0.0643286         |      <span style="color:#8000;">**2.21214**</span>      |
|        to-octets(mpi vs ossl)         |   114.342    |         1275.9          |        0.0935227         |      <span style="color:#8000;">**11.1586**</span>      |
|           add(mpi vs ossl)            |   70.2412    |         221.184         |         0.119319         |      <span style="color:#8000;">**3.14892**</span>      |
|        add-assign(mpi vs ossl)        |   68.6812    |         224.684         |         0.120751         |      <span style="color:#8000;">**3.2714**</span>       |
|           sub(mpi vs ossl)            |    55.061    |         144.502         |         0.134861         |      <span style="color:#8000;">**2.62441**</span>      |
|        sub-assign(mpi vs ossl)        |    59.901    |         224.184         |         0.129288         |      <span style="color:#8000;">**3.74257**</span>      |
|           mul(mpi vs ossl)            |   1604.05    |         9936.55         |        0.0250585         |      <span style="color:#8000;">**6.19467**</span>      |
|           sqr(mpi vs ossl)            |   982.137    |         6909.42         |        0.0321365         |      <span style="color:#8000;">**7.03509**</span>      |
|           div(mpi vs ossl)            |   3040.61    |         31350.4         |        0.0181808         |      <span style="color:#8000;">**10.3106**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.86249e+06  |       1.18812e+07       |       0.000540703        |      <span style="color:#8000;">**3.07604**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.53697e+06  |       1.05303e+08       |       0.000416888        |      <span style="color:#8000;">**12.3349**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17931e+07  |       1.12035e+08       |       0.000350531        |        <span style="color:#8000;">**9.5**</span>        |
|          MUL2(a * 2 = a + a)          |   57.8998    |      <span style="font-style:italic;">No Reference</span>       |         0.131429         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   75.0196    |      <span style="font-style:italic;">No Reference</span>       |         0.116288         |        N/A        |
