# libmpi

[![license](https://img.shields.io/badge/license-Apache-brightgreen.svg?style=flat)](https://github.com/vxfury/libmpi/blob/master/LICENSE)
[![CI Status](https://github.com/vxfury/libmpi/workflows/ci/badge.svg)](https://github.com/vxfury/libmpi/actions)
[![codecov](https://codecov.io/gh/vxfury/libmpi/branch/main/graph/badge.svg?token=5IfLTTEcnF)](https://codecov.io/gh/vxfury/libmpi)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/vxfury/libmpi?color=red&label=release)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](https://github.com/vxfury/libmpi/pulls)

Multiple Precision Integer and Relevant Algorithms, such as Bignum, RSA, DH, ECDH, ECDSA
## Benchmark(libmpi VS openssl)

| brief | average time<br>(nanoseconds) | instability<br>(coefficient of variation) | rating | 
| :-- | :-: | :-: | :-: |
| from-string(mpi vs openssl) | 1597.9<br>21745.7* | 0.0270626 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.6089<br>(Tu es mon meilleur frère...)</span> | 
| to-string(mpi vs openssl) | 710.489<br>1763.42* | 0.0461712 | <span style="color:#008000;font-weight:bold;">2.48198<br>(Tu peux faire mieux, continue)</span> | 
| from-octets(mpi vs openssl) | 140.843<br>454.211* | 0.0905271 | <span style="color:#008000;font-weight:bold;">3.22496<br>(Tu peux faire mieux, continue)</span> | 
| to-octets(mpi vs openssl) | 72.8298<br>981.715* | 0.155258 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">13.4796<br>(Tu es mon meilleur frère...)</span> | 
| add(mpi vs openssl) | 34.6466<br>136.897* | 0.181786 | <span style="color:#008000;font-weight:bold;">3.95125<br>(Tu peux faire mieux, continue)</span> | 
| add-assign(mpi vs openssl) | 32.8812<br>145.938* | 0.196794 | <span style="color:#008000;font-weight:bold;">4.43835<br>(Tu peux faire mieux, continue)</span> | 
| sub(mpi vs openssl) | 33.3782<br>102.317* | 0.218124 | <span style="color:#008000;font-weight:bold;">3.06538<br>(Tu peux faire mieux, continue)</span> | 
| sub-assign(mpi vs openssl) | 35.5604<br>145.181* | 0.204162 | <span style="color:#008000;font-weight:bold;">4.08265<br>(Tu peux faire mieux, continue)</span> | 
| mul(mpi vs openssl) | 1378.1<br>11221* | 0.0325392 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">8.14232<br>(C'est super, dessine-toi une tarte)</span> | 
| sqr(mpi vs openssl) | 763.34<br>7564.46* | 0.0379798 | <span style="color:#008000;font-weight:bold;text-decoration:blink;">9.90968<br>(C'est super, dessine-toi une tarte)</span> | 
| MUL2(a * 2 = a + a) | 42.6234 | 0.310691 | <span style="font-style:italic;">N/A</span> | 
| MUL2(a * 2 = a << 1) | 55.7856 | 0.220549 | <span style="font-style:italic;">N/A</span> | 
