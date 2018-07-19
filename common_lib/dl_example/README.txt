사용법
1. dl을 관리하는 폴더를 만든다.
   $ mkdir -p /etc/3team
2. dl.rc를 참고하여 rc 파일을 만든다.
3. dl.rc를 만든 폴더에 위치한다.
   $ mv dl.rc /etc/3team
4. info.h를 인클루드하여 파일을 빌드한다.
   (반드시 인자로 info_t를 받아야함)
5. make
6. make install
