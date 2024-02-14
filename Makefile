f = main

compose:
	@echo "Running in Docker Compose\n"
	@docker-compose up --build

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