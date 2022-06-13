# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

| description | average time | average time(reference) | coefficient of variation | perfermance ratio | 
| :-: | :-: | :-: | :-: | :-: |
| from-string(mpi vs ossl) | 2383.2 | 30280.2 | 0.0297621 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">12.7057</span> | 
| to-string(mpi vs ossl) | 1203.27 | 4180.33 | 0.0814362 | <span style="color:#008000;font-weight:bold;">3.47415</span> | 
| from-octets(mpi vs ossl) | 242.482 | 716.505 | 0.0670178 | <span style="color:#008000;font-weight:bold;">2.95488</span> | 
| to-octets(mpi vs ossl) | 137.361 | 1576.13 | 0.0853316 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">11.4744</span> | 
| add(mpi vs ossl) | 49.8002 | 265.422 | 0.14171 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.32973</span> | 
| add-assign(mpi vs ossl) | 50.3004 | 271.262 | 0.141004 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.39284</span> | 
| sub(mpi vs ossl) | 62.9404 | 170.621 | 0.126414 | <span style="color:#008000;font-weight:bold;">2.71084</span> | 
| sub-assign(mpi vs ossl) | 57.8804 | 297.942 | 0.131485 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">5.14755</span> | 
| mul(mpi vs ossl) | 3192.68 | 12758 | 0.493956 | <span style="color:#008000;font-weight:bold;">3.99602</span> | 
| sqr(mpi vs ossl) | 1553.13 | 10184.3 | 0.422474 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">6.55728</span> | 
| MUL2(a * 2 = a + a) | 46.1402 | <span style="font-style:italic;">No Reference</span> | 0.147228 | N/A | 
| MUL2(a * 2 = a << 1) | 93.0206 | <span style="font-style:italic;">No Reference</span> | 0.104145 | N/A | 
