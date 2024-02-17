f = main

run:
	@echo "Running in Docker\n"
	@cd app && docker build -t cpp . && docker run -it --rm cpp

local:
	@echo "Compiling..."
	@g++ -std=c++20 -o bin/$(f) app/$(f).cpp -lcurl

	@echo "Running code\n"
	@./bin/$(f)

amend:
	@git add . && git commit --amend --no-edit && git push -f

serve:
	@cd server && docker build -t server . && docker run -it --rm -p 3000:3000 server