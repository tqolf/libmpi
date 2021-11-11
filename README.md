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
|       from-string(mpi vs ossl)        |   2070.83    |         25854.6         |         0.110976         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.4851</span>      |
|        to-string(mpi vs ossl)         |   778.425    |         2494.56         |         0.20657          |      <span style="color:#008000;font-weight:bold;">3.20462</span>      |
|       from-octets(mpi vs ossl)        |   226.222    |         675.925         |         0.34411          |      <span style="color:#008000;font-weight:bold;">2.98789</span>      |
|        to-octets(mpi vs ossl)         |   175.801    |         1206.81         |         0.544576         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.86462</span>      |
|           add(mpi vs ossl)            |   72.0606    |         274.182         |         0.720779         |      <span style="color:#008000;font-weight:bold;">3.80488</span>      |
|        add-assign(mpi vs ossl)        |   81.1604    |         239.122         |         0.810454         |      <span style="color:#008000;font-weight:bold;">2.94628</span>      |
|           sub(mpi vs ossl)            |   119.001    |         219.961         |         0.645517         |      <span style="color:#008000;">1.8484</span>       |
|        sub-assign(mpi vs ossl)        |   84.9806    |         257.562         |         0.769472         |      <span style="color:#008000;font-weight:bold;">3.03083</span>      |
|           mul(mpi vs ossl)            |   1692.17    |         11054.2         |        0.0567111         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.53253</span>      |
|           sqr(mpi vs ossl)            |   1185.55    |         9193.78         |        0.0557882         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.75488</span>      |
|           div(mpi vs ossl)            |   4140.81    |         34623.5         |        0.0345965         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">8.36152</span>      |
|      gcd_consttime(mpi vs ossl)       | 4.82465e+06  |       1.50139e+07       |        0.0602306         |      <span style="color:#008000;font-weight:bold;">3.11192</span>      |
|      montgomery-exp(mpi vs ossl)      |  8.8742e+06  |       1.14605e+08       |         0.018815         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.9144</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.35402e+07  |       1.2024e+08        |        0.0252169         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">8.88025</span>      |
|          MUL2(a * 2 = a + a)          |   32.0404    |      <span style="font-style:italic;">No Reference</span>       |         0.267626         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   53.3806    |      <span style="font-style:italic;">No Reference</span>       |         0.136923         |        N/A        |
