# ============================
#  설정
# ============================
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -g
TARGET = main

# ============================
#  소스 / 헤더 / 오브젝트
# ============================
SRCS = main.cpp Client.cpp
OBJS = $(SRCS:.cpp=.o)

DEPS = Client.h Support.h

# ============================
#  빌드 규칙
# ============================
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ============================
#  clean
# ============================
clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean

