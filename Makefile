f = main

run:
	@echo "Running in Docker\n"
	@cd app && docker build -t cpp . && docker run -it --rm --env-file .env cpp

local:
	@echo "Compiling..."
	@g++ -std=c++20 -o bin/$(f) app/$(f).cpp -lcurl

	@echo "Running code\n"
	@./bin/$(f)

serve:
	@cd server && docker build -t server . && docker run -it --rm -p 3000:3000 --env-file .env server

local-server:
	@cd server && python3 main.py

amend:
	@git add . && git commit --amend --no-edit && git push -f
