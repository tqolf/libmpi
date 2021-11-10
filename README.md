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
|       from-string(mpi vs ossl)        |     2752     |         30063.7         |         0.198058         |      **<span style="color:#008000;">10.9243</span>**      |
|        to-string(mpi vs ossl)         |   694.584    |         3148.18         |         0.042797         |      **<span style="color:#008000;">4.53246</span>**      |
|       from-octets(mpi vs ossl)        |   238.661    |         717.764         |        0.0652553         |      **<span style="color:#008000;">3.00746</span>**      |
|        to-octets(mpi vs ossl)         |   136.101    |         1534.79         |        0.0926883         |      **<span style="color:#008000;">11.2769</span>**      |
|           add(mpi vs ossl)            |   63.0204    |         379.882         |         0.328442         |      **<span style="color:#008000;">6.02793</span>**      |
|        add-assign(mpi vs ossl)        |   53.8802    |         331.322         |         0.165472         |      **<span style="color:#008000;">6.14923</span>**      |
|           sub(mpi vs ossl)            |   65.0804    |         170.301         |         0.123963         |      **<span style="color:#008000;">2.61678</span>**      |
|        sub-assign(mpi vs ossl)        |   61.2204    |         332.022         |         0.127829         |      **<span style="color:#008000;">5.42339</span>**      |
|           mul(mpi vs ossl)            |   2029.77    |         12021.3         |        0.0258555         |      **<span style="color:#008000;">5.92248</span>**      |
|           sqr(mpi vs ossl)            |   1170.43    |         8590.17         |         0.09854          |      **<span style="color:#008000;">7.33935</span>**      |
|           div(mpi vs ossl)            |   4029.14    |          39953          |        0.0488435         |      **<span style="color:#008000;">9.91599</span>**      |
|      gcd_consttime(mpi vs ossl)       | 5.03578e+06  |       1.5282e+07        |         0.011048         |      **<span style="color:#008000;">3.03469</span>**      |
|      montgomery-exp(mpi vs ossl)      | 1.02376e+07  |       1.30351e+08       |        0.0125288         |      **<span style="color:#008000;">12.7326</span>**      |
| montgomery-exp-consttime(mpi vs ossl) | 1.47717e+07  |       1.39676e+08       |        0.0458294         |      **<span style="color:#008000;">9.45564</span>**      |
|          MUL2(a * 2 = a + a)          |    15.68     |      <span style="font-style:italic;">No Reference</span>       |         0.252551         |        N/A        |
|         MUL2(a * 2 = a << 1)          |   40.4602    |      <span style="font-style:italic;">No Reference</span>       |         0.158401         |        N/A        |
