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
|       from-string(mpi vs ossl)        |   1959.08    |          24006          |        0.0226568         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.2537</span>      |
|        to-string(mpi vs ossl)         |    922.65    |         2191.34         |        0.0330522         |      <span style="color:#008000;font-weight:bold;">2.37505</span>      |
|       from-octets(mpi vs ossl)        |   212.742    |         541.666         |         0.068562         |      <span style="color:#008000;font-weight:bold;">2.54611</span>      |
|        to-octets(mpi vs ossl)         |   114.281    |         1268.09         |        0.0935472         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.0962</span>      |
|           add(mpi vs ossl)            |   65.2606    |         220.583         |         0.123793         |      <span style="color:#008000;font-weight:bold;">3.38003</span>      |
|        add-assign(mpi vs ossl)        |   65.9208    |         224.122         |         0.123167         |      <span style="color:#008000;font-weight:bold;">3.39987</span>      |
|           sub(mpi vs ossl)            |   53.5606    |         141.941         |         0.136648         |      <span style="color:#008000;font-weight:bold;">2.65011</span>      |
|        sub-assign(mpi vs ossl)        |   57.7206    |         222.582         |         0.131667         |      <span style="color:#008000;font-weight:bold;">3.8562</span>       |
|           mul(mpi vs ossl)            |   1611.96    |         9924.05         |        0.0250501         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.15652</span>      |
|           sqr(mpi vs ossl)            |   962.331    |         6955.06         |        0.0323823         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">7.2273</span>       |
|           div(mpi vs ossl)            |   2988.51    |         31310.8         |        0.0183765         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">10.4771</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.38958e+06  |       1.19313e+07       |       0.000638882        |      <span style="color:#008000;font-weight:bold;">3.51999</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.43763e+06  |       1.08388e+08       |        0.00165175        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.8458</span>      |
| montgomery-exp-consttime(mpi vs ossl) | 1.17219e+07  |       1.15252e+08       |       0.000305696        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.83224</span>      |
|          MUL2(a * 2 = a + a)          |   44.9206    |      <span style="font-style:italic;">No Reference</span>       |         0.149206         |        N/A        |
|         MUL2(a * 2 = a << 1)          |    62.181    |      <span style="font-style:italic;">No Reference</span>       |         0.126825         |        N/A        |
