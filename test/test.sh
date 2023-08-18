#!/bin/zsh

PORT_NGINX=8670
PORT_PERSO=8694

BOLD='\033[1m'

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

MOVE_UP='\033[1F'
ERASE_LINE='\033[2K'

rm -rf test/results

for request in test/requests/*; do
	request_name=${request}
	request_name=${request_name##*/}
	request_name=${request_name%.*}
	echo "${YELLOW}⌛ ${request_name} in progress${NC}"
	cat ${request} | nc 127.0.0.1 ${PORT_NGINX} -C -w 1 > test/output_nginx
	cat test/output_nginx | grep -v -E "Date:|Server:|Last-Modified:" > test/output_2_nginx
	cat ${request} | nc 127.0.0.1 ${PORT_PERSO} -C -w 1 > test/output_perso
	cat test/output_perso | grep -v -E "Date:|Server:" > test/output_2_perso
	diff test/output_2_nginx test/output_2_perso > test/output_2_diff
	echo "${MOVE_UP}${ERASE_LINE}${MOVE_UP}"
	if [ -s test/output_2_diff ]
	then
		echo "${RED}❌ ${request_name} is different from nginx${NC}"
		request_folder="test/results/${request_name}/"
		mkdir -p ${request_folder}
		cp test/output_2_diff ${request_folder}diff
		cp test/output_2_perso ${request_folder}perso
		cp test/output_2_nginx ${request_folder}nginx

		PID=$(ps -ax | grep -F "./webserv" | grep -v "grep" | awk '{print $$1}')
		if [ -z "${PID}" ]
		then 
			echo "${RED}${BOLD}💥 Your program crashed (oupsi)${NC}"
			break
		fi
	else
		echo "${GREEN}✅ ${request_name} perfectly flawless${NC}"
	fi
done

rm -f test/output_*
