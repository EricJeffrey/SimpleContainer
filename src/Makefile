build : main.cpp
	mkdir ../build/
	g++ -o ../build/main.out main.cpp -lcap

.PHONY : cleansu
cleansu:
	-sudo rm ../build/main.out
	-sudo rm -r /home/eric/coding/simp_containers/alpine_container

runsu: 
	make cleansu
	make build
	sudo ../build/main.out
