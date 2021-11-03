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
|       from-string(mpi vs ossl)        |   2145.29    |         24183.8         |        0.0257775         |      <span style="color:#8000;">**11.273**</span>       |
|        to-string(mpi vs ossl)         |   589.687    |         2283.17         |        0.0683512         |      <span style="color:#8000;">**3.87183**</span>      |
|       from-octets(mpi vs ossl)        |   254.383    |         550.826         |         0.140384         |      <span style="color:#8000;">**2.16534**</span>      |
|        to-octets(mpi vs ossl)         |   129.382    |         1288.68         |         0.248054         |      <span style="color:#8000;">**9.96027**</span>      |
|           add(mpi vs ossl)            |    86.301    |         221.243         |         0.362268         |      <span style="color:#8000;">**2.56362**</span>      |
|        add-assign(mpi vs ossl)        |    83.641    |         239.023         |         0.378755         |      <span style="color:#8000;">**2.85773**</span>      |
|           sub(mpi vs ossl)            |   70.0408    |         158.382         |         0.448536         |      <span style="color:#8000;">**2.26128**</span>      |
|        sub-assign(mpi vs ossl)        |    75.501    |         239.963         |         0.414322         |      <span style="color:#8000;">**3.17827**</span>      |
|           mul(mpi vs ossl)            |     1620     |         9895.3          |        0.0309634         |      <span style="color:#8000;">**6.10821**</span>      |
|           sqr(mpi vs ossl)            |   1002.61    |         6952.72         |        0.0387235         |      <span style="color:#8000;">**6.93461**</span>      |
|           div(mpi vs ossl)            |   3397.76    |         32193.5         |        0.0207593         |      <span style="color:#8000;">**9.47492**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.86668e+06  |       1.20248e+07       |       0.000524077        |      <span style="color:#8000;">**3.10986**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.58149e+06  |       1.06889e+08       |       0.000512054        |      <span style="color:#8000;">**12.4558**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17893e+07  |       1.13047e+08       |        0.0008217         |       <span style="color:#8000;">**9.589**</span>       |
|          MUL2(a * 2 = a + a)          |   45.7206    |      <span style="font-style:italic;">No Reference</span>       |         0.147927         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   60.9806    |      <span style="font-style:italic;">No Reference</span>       |         0.128063         |        N/A        |
