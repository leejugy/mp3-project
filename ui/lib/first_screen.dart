import 'package:flutter/material.dart';
import 'dart:async';
import './next_screen.dart';

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class Tick {
  static String padInt(int val, int pad) => val.toString().padLeft(pad, '0');
  static String getCurrentTime() {
    String currentTime = "";
    DateTime now = DateTime.now();
    currentTime =
        '${padInt(now.year, 4)}-${padInt(now.month, 2)}-${padInt(now.day, 2)} '
        '${padInt(now.hour, 2)}:${padInt(now.minute, 2)}:${padInt(now.second, 2)}';
    return currentTime;
  }
}

class _MyAppState extends State<MyApp> {
  AppBar topBar() {
    return AppBar(
      title: Container(
        height: 100,
        color: Colors.blue,
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Text('Current Time : ${Tick.getCurrentTime()}'),
          ],
        ),
      ),
    );
  }

  int middleStringCase = 0;
  int middleButtonIconCase = 0;

  List<String> middleStringList = [
    "hellow there",
    "come on",
    "wake up guys!",
    "it's time to change!",
  ];

  List<Widget> middleButtonIconList = [
    Icon(Icons.radio_button_off),
    Icon(Icons.radio_button_checked),
  ];

  Widget middleButton() {
    return ElevatedButton(
      onPressed: () {
        setState(() {
          middleStringCase = (middleStringCase + 1) & 3;
          middleButtonIconCase = (middleButtonIconCase + 1) & 1;
        });
      },
      child: Row(
        children: [
          middleButtonIconList[middleButtonIconCase],
          Text('presse me'),
        ],
      ),
    );
  }

  Widget middleBar() {
    return SizedBox(
        child: Center(
      child: Row(
        mainAxisAlignment: MainAxisAlignment.spaceEvenly,
        children: [
          middleButton(),
          Text('change string : ${middleStringList[middleStringCase]}'),
        ],
      ),
    ));
  }

  Widget buttomButton(BuildContext context) {
    return ElevatedButton(
      onPressed: () {
        Navigator.push(
          context,
          PageRouteBuilder(
            pageBuilder: (_, __, ___) => const NextPage(),
            transitionDuration: Duration.zero, // 전환 시간 0
            reverseTransitionDuration: Duration.zero, // 뒤로 갈 때도 0
          ),
        );
      },
      child: Text(
        'next page',
        style: TextStyle(
          color: Colors.red,
          fontWeight: FontWeight.bold,
          fontSize: 16,
        ),
      ),
    );
  }

  Widget bottomBar(BuildContext context) {
    return BottomAppBar(
      child: Center(
        child: Row(
          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
          children: [
            Text('bottom line'),
            buttomButton(context),
          ],
        ),
      ),
    );
  }

  Timer? _timer;

  @override
  void initState() {
    super.initState();
    // 1초마다 화면만 새로 그리기
    _timer = Timer.periodic(
      const Duration(seconds: 1),
      (_) {
        if (mounted) setState(() {}); // _Tick.getCurrentTime()을 다시 읽게 됨
      },
    );
  }

  @override
  void dispose() {
    _timer?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      home: Scaffold(
        appBar: topBar(),
        body: middleBar(),
        bottomNavigationBar: bottomBar(context),
      ),
    );
  }
}
