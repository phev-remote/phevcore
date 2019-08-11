# Phev Remote Core Library 

A library that can be used to communicate with the Mistushish Outlander PHEV with the Remote WiFi option in the MMC so mainly the GH4 + models.

Device registration and connecting to the car is functional as well as being able to read the car data and swtich on the lights and manual air conditioning.

More features will be avalable soon.

It requires that the device is connected to the same network as the car, the 192.168.8.x subnet.

## License

MIT License

>  Copyright (c) 2019 Jamie Nuttall
>
>  Permission is hereby granted, free of charge, to any person obtaining a copy
>  of this software and associated documentation files (the "Software"), to deal
>  in the Software without restriction, including without limitation the rights
>  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
>  copies of the Software, and to permit persons to whom the Software is
>  furnished to do so, subject to the following conditions:
>
>  The above copyright notice and this permission notice shall be included in
>  all copies or substantial portions of the Software.
>
>  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
>  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
>  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
>  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
>  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
>  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
>  THE SOFTWARE.

### pre-reqs

#### Messaging core library

```
git clone https://github.com/papawattu/msg-core
cd msg-core
mkdir -p build
cd build
cmake ..
make
sudo make install
```
#### cJSON
```
git clone https://github.com/DaveGamble/cJSON
cd cJSON
mkdir -p build
cd build
cmake ..
make
sudo make install
```
### Build instructions
```
git clone https://github.com/phev-remote/phevcore
cd phevcore
mkdir -p build
cd build
cmake ..
make
sudo make install
```
### The CLI 

Has been tested on a raspberry pi and other linux systems can be found here.

https://github.com/phev-remote/phevcli
