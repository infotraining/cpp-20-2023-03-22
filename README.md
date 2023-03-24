# Szkolenie - C++20 #

## 

Ankieta 

* https://login.microsoftonline.com/92e84ceb-fbfd-47ab-be52-080c6b87953f/oauth2/v2.0/authorize?client_id=3e62f81e-590b-425b-9531-cad6683656cf&scope=https%3A%2F%2Fservice.powerapps.com%2F%2F.default%20openid%20profile%20offline_access&redirect_uri=https%3A%2F%2Fapps.powerapps.com%2Fauth%2Fv2&client-request-id=9ea3f46c-71ab-49d2-8012-3758928989ad&response_mode=fragment&response_type=code&x-client-SKU=msal.js.browser&x-client-VER=2.18.0&x-client-OS=&x-client-CPU=&client_info=1&code_challenge=9ZWNVYFoeQWj2ZVEHXCVbMoNdCrLtcjMLzMFGWbHQ08&code_challenge_method=S256&nonce=cb7ef4f0-7cb6-4b7c-b049-3d554d63c932&state=eyJpZCI6ImQ4ZTQ5ZmE0LThhNDctNDI3MC04NDBjLTYxMWNjYjI5MTUxOSIsIm1ldGEiOnsiaW50ZXJhY3Rpb25UeXBlIjoicmVkaXJlY3QifX0%3D

* https://forms.gle/gyhTU74Xz5P3wdeC9

## Slajdy

* https://infotraining.bitbucket.io/cpp-20

## Konfiguracja środowiska

### Lokalna

Przed szkoleniem należy zainstalować na swoim:

#### Kompilator + CMake

* Dowolny kompilator C++ (gcc, clang, Visual C++) wspierający C++20
  * zalecane: gcc-12.2+, Visual Studio 2022

* [CMake > 3.16](https://cmake.org/)
  * proszę sprawdzić wersję w lini poleceń

  ```
  cmake --version
  ```

#### Visual Studio Code

* [Visual Studio Code](https://code.visualstudio.com/)
* Zainstalować wtyczki
  * C/C++ Extension Pack
  * Live Share

### Docker + Visual Studio Code

Jeśli uczestnicy szkolenia korzystają w pracy z Docker'a, to należy zainstalować:

#### Visual Studio Code

* [Visual Studio Code](https://code.visualstudio.com/)
* Zainstalować wtyczki
  * Live Share
  * Dev Containers ([wymagania](https://code.visualstudio.com/docs/devcontainers/containers#_system-requirements))
    * po instalacji wtyczki - należy otworzyć w VS Code folder zawierający sklonowane repozytorium i
      z palety poleceń (Ctrl+Shift+P) wybrać opcję **Dev Containers: Rebuild and Reopen in Container**

