f = main

run:
	@echo "Running in Docker\n"
	@docker build -t cpp .
	@docker run -it --rm cpp

local:
	@echo "Compiling..."
	@g++ -std=c++20 -o bin/$(f) $(f).cpp -lcurl

	@echo "Running code\n"
	@./bin/$(f)

amend:
	@git add . && git commit --amend --no-edit && git push -f

serve-l:
	@cd server && go run main.go

serve:
	@cd server && docker build -t go-server . && docker run -it --rm -p 3000:3000 go-server