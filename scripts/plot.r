#!/usr/bin/env Rscript

library("ggplot2")
library("dplyr")

dev.new(width=5, height=4)


legacy.data <- read.csv(file = "legacy_data.csv",
                        stringsAsFactors = TRUE)

anydsl.data <- read.csv(file = "anydsl_data.csv",
                        stringsAsFactors = TRUE)

anydsl.plot <- ggplot(anydsl.data, aes(y=median,x=backend,group=backend)) +
  geom_bar(stat = "identity", aes(fill=ifelse(generic, "Yes", "No")), position = position_dodge2(padding = .2), alpha = 0.5) +
  scale_fill_manual(values = c("red", "blue")) +
  labs(title="AnyDSL - Median(250) Runtime (ms, no I/O)", y="", x="Backends", fill="Generic")

legacy.plot <- ggplot(legacy.data, aes(y=median,x=backend,group=backend)) +
  geom_bar(stat = "identity", aes(fill=ifelse(generic, "Yes", "No")), position = position_dodge2(padding = .2), alpha = 0.5) +
  scale_fill_manual(values = c("red", "blue")) +
  labs(title="Legacy - Median(250) Runtime (ms, no I/O)", y="", x="Backends", fill="Generic")

ggsave(filename = "anydsl-performance.svg", plot = anydsl.plot)
ggsave(filename = "legacy-performance.svg", plot = legacy.plot)


#legacy.generic_data <- subset(legacy.data, generic == 1)
#anydsl.generic_data <- subset(subset(anydsl.data, generic == 1), backend == "cpu")
#plot <- ggplot(NULL) +
#  geom_line(data = anydsl.generic_data,
#            aes(y=median,x=threads,group=1),
#            color="red") +
#  geom_point(data = anydsl.generic_data, aes(y=median,x=threads,color="AnyDSL")) +
#  geom_line(data = legacy.generic_data,
#            aes(y=median,x=threads,group=1),
#            color="blue") +
#  geom_point(data = legacy.generic_data, aes(y=median,x=threads,color="Legacy")) +
#  labs(title="Legacy vs. AnyDSL - Median(250) Runtime (ms, no I/O)", y="", x="Threads")
#
#ggsave(filename = "anydsl-legacy-cpu.svg", plot = plot)




legacy.data <- read.csv(file = "legacy_raw_data.csv",
                        stringsAsFactors = TRUE)

anydsl.data <- read.csv(file = "anydsl_raw_data.csv",
                        stringsAsFactors = TRUE)

legacy.data[,ncol(legacy.data)+1] <- "Legacy"
anydsl.data[,ncol(anydsl.data)+1] <- "AnyDSL"
names(legacy.data)[ncol(legacy.data)] <- "program"
names(anydsl.data)[ncol(anydsl.data)] <- "program"

complete.data <- rbind(legacy.data, anydsl.data)
complete.data <- subset(complete.data, generic == 0)
complete.data$threads <- as.factor(complete.data$threads)

plot <- ggplot(data = complete.data) +
  geom_violin(trim = FALSE,  mapping = aes(x=threads,y=median,fill=program)) +
  geom_boxplot(width = 0.9, position = position_dodge2(padding = 0.9),
               mapping = aes(x=threads,y=median,fill=program), show.legend = FALSE) +
  labs(title="Legacy vs AnyDSL - Runtime(100 runs/thread) (ms, no I/O)", y="", x="Backends", fill="Program")

ggsave(filename = "anydsl-legacy-cpu-violin.svg", plot = plot)

