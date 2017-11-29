all: link

link: manager.cpp manager.h router.cpp router.h
	g++ -I. -Wall -std=c++11 -pthread -g router.cpp -o router
	g++ -I. -Wall -std=c++11 -pthread -g manager.cpp -o manager
router:
	g++ -I. -Wall -std=c++11 -pthread -g router.cpp -o router
manager:
	g++ -I. -Wall -std=c++11 -pthread -g manager.cpp -o manager
clean:
	rm -f *.o *.tar *.out router manager
tar:
	tar -cvf link.tar manager.cpp manager.h router.cpp router.h makefile README.txt
