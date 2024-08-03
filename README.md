![20240803_131214](https://github.com/user-attachments/assets/8ed0e75a-d947-47c5-9d72-7d17b410ce33)
![20240803_131117](https://github.com/user-attachments/assets/59e0317e-22f7-48bf-8061-88339580a26b)

此项目使用ESP32+OLED实现电脑副屏功能，开机时实时显示CPU、RAM、GPU占用率（API由AIDA64提供）。关机时显示时钟信息。

---

准备阶段：
硬件：
* ESP32芯片*1，¥25.25，淘宝链接：https://item.taobao.com/item.htm?spm=a1z09.2.0.0.2f872e8dKRr4kt&id=672317019032&_u=3mog2lbab2&sku_properties=1627207:1764709649
* OLED2.42寸 5针 3.3V 白色*1，¥95.2，淘宝链接：https://detail.tmall.com/item.htm?id=558395483864&spm=a1z09.2.0.0.14fc2e8d1f7pi5&_u=3mog2l2f55&pisk=fWOE0qj0rXheYKvlbG1P78Wtst1dgsnjqQs5rUYlRMjnewGuaU8cP2n-Jd7kVh3-J6Tl43-1X7_Sp0FybHTDd9tWdgbljhYQOgGdU38XD3NS9p_uahTRE3Oyy77kr33dV2H6JeCRZmif4jTpJ_16ARdPKN4GkFV3-70t_b0NZmijaRaGpD1kFOATEGWGXa7ut7xus1bdja2hZ3jGIZbbx_fkqF0NrZqlr7juIlb5Pby6qH3N2902fPYtbqVOKiYhxetWs86gpFjN5wOO8FSD-8FuZC7FLQsRPmiRdLYAHBT-a5CBudfGlehU_sYhuHs6mjVNsFTMVtpt2S69bQWkt9UomiXPYdAhQzVkW_SFHL-EYJIHdhWASO4rMNtfjedHQ4EM-nsFtNXsiq8GEdOdHBigTsvvR69k2xwOTUvh_gy42NcfMQpUq8WhWNSj7Vci6macZ5KBU8eRQAQNc4Vue8BhWNSj7Vy8eORO7iguZ&skuId=4541441894915
* 杜邦线*4，根据机箱需要设置长度，价格忽略不计。
* USB-TypeC线*1，根据机箱需要设置长度，价格忽略不计。
* 扩展板9针USB主板内置USB口*1(不必需），¥8，淘宝链接：https://detail.tmall.com/item.htm?id=736189528051&spm=a1z09.2.0.0.14fc2e8d1f7pi5&_u=3mog2l166e&pisk=f7Vj0zqxYnxj-BUSjxQrFVGUvdGslo1F6FgT-Pd2WjhxCCU4AVRZQt0610adomWGnlw7XuDqDckZ1VZEqrRNor711PUKDfyq0VEzfuv405oa41a3AxR4W580EkzpgSWm_CGmsfIFY65Eurci6ak_g4Ymw20pX1ovH3pj0r6dY65UPt3m_zIFSzLn1c3Z6qHtD_QSo2mt6A3Yyagt7np9MlURPVmJMCdtBU3-R20xWbupVV-jzr_JLXYwmhpum0O9OemnHZqvQIdv5qDxvYEW64PSlxiLkSXc9m08a7MEUFS-wy2zD4GCaKhLPREQplXeEjaTHlD7mgvS48F0WcHXWBz7VPh8MJT96zGjmYFxUnsSvS48tbHP9wa8Zoz0NP8O64rE2rVx16QnGjnxN4VNqIn_eRFqnfxAYx4TFugj9gW9TDGExdTSKCgSYa_WIduiI1x56ogAex3ok675P3PiH40SYa_WIdDxrqeFPatzs&skuId=5085334564172

软件：
* AIDA64，¥1.88可激活，淘宝链接：https://item.taobao.com/item.htm?spm=a1z09.2.0.0.14fc2e8d1f7pi5&id=625367245693&_u=3mog2ld188&pisk=ffN-0UVSsijkeF63Nz6mtLvhuV7c27Uzuuz6tDmkdoEYvPLuEvmndXEaXvOhUbDK9oi0Z04oN2MQxlwoV8mH9WE42QAnNkfKAPyV800uZe3Qhud3EzmuHehzi8AnqglLRlcdjGfGszzrQXsGjt4pvCcrJL9IRHmfG2KTPUY1szzrYUKCS-1Mp5d-37gQAYMjc4mXNQM7AxOj8mOSRQgBcZ3EcDGIVDTjh4u6d3MSdbWOym5S7BeQU3sMS1Q60nPxlMk0ynp-jWuRCcU-DGtLMCk-fzivOGDijroK05scHrraFu04v1IxToyQA2EXcMDzR-Exyuj9YDVzrWnuyTsbkjo8Rvh9R_axw0H_ZALBTlaYRRDYiN1Zwbi7KbzHWg4Ywu4zMyYBhbhuHAF-11-Sx-4T92FFYsET7Srxll1dVg8us5HX-hmt-L_AkpJEFqDiZXR5nQAq1qnGPY9eLAFqkcbAkpJEFquxjaOWLpkTg
* Arduino，免费。

---

操作阶段：
* 1.电脑设置AIDA64，参考文献：https://blog.csdn.net/weixin_42487906/article/details/119990801?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522172266571116800182194010%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&request_id=172266571116800182194010&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-119990801-null-null.142^v100^pc_search_result_base8&utm_term=aida64%20esp32&spm=1018.2226.3001.4187注意如图勾选5个输出项：![QQ_1722665863713](https://github.com/user-attachments/assets/c69debdd-3e30-4d57-aa72-8cedd70b21ef)
* 2.连接硬件，ESP32-OLED：SCL-GPIO21，SDA-GPIO22，VCC-3.3V，GND-GND。
* 3.写入代码。
