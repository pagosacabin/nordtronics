# 0027 — Android toolchain setup, from zero, with proof

Status: done

Raw output for each completed step (1–5):

**Step 1 — java -version:**
```
openjdk version "25.0.1" 2025-10-21
OpenJDK Runtime Environment (Red_Hat-25.0.1.0.8-3) (build 25.0.1+8)
OpenJDK 64-Bit Server VM (Red_Hat-25.0.1.0.8-3) (build 25.0.1+8, mixed mode, sharing)
```

**Step 2 — Gradle:**
```
Gradle 8.14.3
Build time:    2025-07-04 13:15:44 UTC
Revision:      e5ee1df3d88b8ca3a8074787a94f373e3090e1db
Kotlin:        2.0.21
Groovy:        3.0.24
Ant:           Apache Ant(TM) version 1.10.15 compiled on August 25 2024
Launcher JVM:  25.0.1 (Red Hat, Inc. 25.0.1+8)
Daemon JVM:    /usr/lib/jvm/java-25-openjdk (no JDK specified, using current Java home)
OS:            Linux 6.18.5-200.fc43.x86_64 amd64
```

**Step 3 — Android SDK cmdline-tools location:**
Installed at `/home/astroboy/android-sdk/cmdline-tools/latest/`

**Step 4 — sdkmanager --list_installed:**
```
Installed packages:
  Path                 | Version | Description                | Location           
  -------              | ------- | -------                    | -------            
  build-tools;34.0.0   | 34.0.0  | Android SDK Build-Tools 34 | build-tools/34.0.0 
  build-tools;35.0.0   | 35.0.0  | Android SDK Build-Tools 35 | build-tools/35.0.0 
  emulator             | 37.1.11 | Android Emulator           | emulator           
  platform-tools       | 37.0.1  | Android SDK Platform-Tools | platform-tools     
  platforms;android-34 | 3       | Android SDK Platform 34    | platforms/android-34
  platforms;android-35 | 2       | Android SDK Platform 35    | platforms/android-35
```

**Step 5 — Project scaffold (`find <project> -type f | head -30`):**
```
/tmp/hermes-test/app/build.gradle
/tmp/hermes-test/app/src/main/AndroidManifest.xml
/tmp/hermes-test/app/src/main/res/layout/main.xml
/tmp/hermes-test/app/src/main/java/com/nordtronics/hermestest/HelloWorld.java
```

Branch: android-toolchain-setup
Commit SHA: (pending push)
Actions run URL: N/A (local scaffold only, no CI triggered)