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
|       from-string(mpi vs ossl)        |   2109.54    |         28790.5         |        0.0218529         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.6478</span>      |
|        to-string(mpi vs ossl)         |   945.359    |         2890.42         |        0.0536279         |      <span style="color:#008000;font-weight:bold;">3.05748</span>      |
|       from-octets(mpi vs ossl)        |    229.94    |         772.739         |         0.066696         |      <span style="color:#008000;font-weight:bold;">3.36062</span>      |
|        to-octets(mpi vs ossl)         |    105.44    |         1505.36         |        0.0973874         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">14.2769</span>      |
|           add(mpi vs ossl)            |    42.08     |         291.36          |         0.154533         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.92394</span>      |
|        add-assign(mpi vs ossl)        |    41.02     |         294.32          |         0.157488         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.17503</span>      |
|           sub(mpi vs ossl)            |   51.4398    |         166.08          |         0.139442         |      <span style="color:#008000;font-weight:bold;">3.22862</span>      |
|        sub-assign(mpi vs ossl)        |   50.3198    |         293.24          |         0.141309         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.82752</span>      |
|           mul(mpi vs ossl)            |   1898.32    |         12178.5         |        0.0230283         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.41544</span>      |
|           sqr(mpi vs ossl)            |   1070.24    |         8466.81         |         0.03146          |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.91114</span>      |
|           div(mpi vs ossl)            |   2472.52    |         38086.9         |        0.0204393         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">15.4041</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.73763e+06  |       1.42577e+07       |       0.000978295        |      <span style="color:#008000;font-weight:bold;">3.81465</span>      |
|      montgomery-exp(mpi vs ossl)      | 9.81124e+06  |       1.28208e+08       |       0.000887248        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.0675</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.35088e+07  |       1.35296e+08       |       0.000899647        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">10.0154</span>      |
|          MUL2(a * 2 = a + a)          |   38.5402    |      <span style="font-style:italic;">No Reference</span>       |         0.161403         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   75.2802    |      <span style="font-style:italic;">No Reference</span>       |         0.115259         |        N/A        |
