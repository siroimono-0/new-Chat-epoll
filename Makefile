# 컴파일러 설정
CXX = g++
CXXFLAGS = -Wall -Wextra -g
TARGET = main

# 소스 파일 목록
SRCS = main.cpp testServer.cpp
OBJS = $(SRCS:.cpp=.o)

# 기본 빌드 규칙
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 개별 .cpp → .o 규칙
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 청소 (빌드 파일 삭제)
clean:
	rm -f $(OBJS) $(TARGET)
