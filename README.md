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
|       from-string(mpi vs ossl)        |   2145.39    |         24309.8         |        0.0257503         |      <span style="color:#8000;">**11.3312**</span>      |
|        to-string(mpi vs ossl)         |   589.187    |         2402.71         |        0.0657319         |      <span style="color:#8000;">**4.07801**</span>      |
|       from-octets(mpi vs ossl)        |   253.003    |         551.147         |         0.146043         |      <span style="color:#8000;">**2.17842**</span>      |
|        to-octets(mpi vs ossl)         |   129.482    |         1291.14         |         0.249322         |      <span style="color:#8000;">**9.97158**</span>      |
|           add(mpi vs ossl)            |   84.8212    |         221.063         |         0.372705         |      <span style="color:#8000;">**2.60622**</span>      |
|        add-assign(mpi vs ossl)        |    83.721    |         224.783         |         0.377379         |      <span style="color:#8000;">**2.6849**</span>       |
|           sub(mpi vs ossl)            |   70.3808    |         158.422         |         0.457816         |      <span style="color:#8000;">**2.25093**</span>      |
|        sub-assign(mpi vs ossl)        |    76.401    |         224.183         |         0.404693         |      <span style="color:#8000;">**2.93429**</span>      |
|           mul(mpi vs ossl)            |   1620.22    |         9915.74         |        0.0313067         |       <span style="color:#8000;">**6.12**</span>        |
|           sqr(mpi vs ossl)            |   980.412    |         6988.91         |        0.0445212         |      <span style="color:#8000;">**7.12854**</span>      |
|           div(mpi vs ossl)            |   3360.42    |         31292.7         |        0.0172761         |      <span style="color:#8000;">**9.31213**</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.87758e+06  |       1.19591e+07       |       0.000608955        |      <span style="color:#8000;">**3.08418**</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.43999e+06  |       1.08121e+08       |         0.001346         |      <span style="color:#8000;">**12.8105**</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17611e+07  |       1.1528e+08        |       0.000309179        |      <span style="color:#8000;">**9.80187**</span>      |
|          MUL2(a * 2 = a + a)          |   23.1204    |      <span style="font-style:italic;">No Reference</span>       |         0.207978         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   40.4204    |      <span style="font-style:italic;">No Reference</span>       |         0.157339         |        N/A        |
