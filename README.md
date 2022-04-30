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
|       from-string(mpi vs ossl)        |   2162.06    |         24274.3         |        0.0343297         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.2274</span>      |
|        to-string(mpi vs ossl)         |   604.045    |         2292.14         |        0.0995042         |      <span style="color:#008000;font-weight:bold;">3.79465</span>      |
|       from-octets(mpi vs ossl)        |   263.882    |         549.505         |         0.197302         |      <span style="color:#008000;font-weight:bold;">2.08239</span>      |
|        to-octets(mpi vs ossl)         |   139.021    |         1294.45         |         0.357622         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.31118</span>      |
|           add(mpi vs ossl)            |   91.2608    |         221.002         |         0.501623         |      <span style="color:#008000;font-weight:bold;">2.42165</span>      |
|        add-assign(mpi vs ossl)        |   92.3208    |         225.482         |         0.547145         |      <span style="color:#008000;font-weight:bold;">2.44238</span>      |
|           sub(mpi vs ossl)            |   77.2408    |         142.161         |         0.589543         |      <span style="color:#008000;">1.8405</span>       |
|        sub-assign(mpi vs ossl)        |   88.1208    |         223.582         |         0.629723         |      <span style="color:#008000;font-weight:bold;">2.53722</span>      |
|           mul(mpi vs ossl)            |   1608.15    |         9885.89         |        0.0250176         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.14735</span>      |
|           sqr(mpi vs ossl)            |   1013.57    |         6899.02         |        0.0323194         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.80666</span>      |
|           div(mpi vs ossl)            |   3355.85    |         31253.9         |         0.017325         |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.31326</span>      |
|      gcd_consttime(mpi vs ossl)       | 3.90851e+06  |       1.18776e+07       |       0.000610784        |      <span style="color:#008000;font-weight:bold;">3.03892</span>      |
|      montgomery-exp(mpi vs ossl)      | 8.58595e+06  |       1.04989e+08       |       0.000736289        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.228</span>       |
| montgomery-exp-consttime(mpi vs ossl) | 1.18585e+07  |       1.11362e+08       |       0.000644693        |      <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.39089</span>      |
|          MUL2(a * 2 = a + a)          |   57.9206    |      <span style="font-style:italic;">No Reference</span>       |         0.131398         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   73.0808    |      <span style="font-style:italic;">No Reference</span>       |         0.116981         |        N/A        |
