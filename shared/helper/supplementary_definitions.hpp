#include <string>

const std::string xmlSupplementaryDefinitions =
R"5UPPL3M3N74RY(

<?xml version='1.0' encoding='utf-8'?>
<ituADM xmlns="urn:metadata-schema:adm">
  <coreMetadata>
    <format>
      <audioFormatExtended>

        <!-- Available layouts in MHAS 5.0.0 -->

        <!-- 3.0 Surr -->
        <!-- ChannelConfiguration 9 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_9 in MPEG-H ADM Profile 1.0.0 -->
        <!-- CICP 9 in MHAS 5.0.0 exports -->
        <audioPackFormat audioPackFormatID="AP_00011101" audioPackFormatName="3.0 Surr (CICP 9)" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010009</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 5.0 + 2H -->
        <audioPackFormat audioPackFormatID="AP_00011102" audioPackFormatName="5.0 + 2H" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000F</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 5.1 + 6H -->
        <!-- ChannelConfiguration 17 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_17 in MPEG-H ADM Profile 1.0.0 -->
        <!-- CICP 17 in MHAS 5.0.0 exports -->
        <audioPackFormat audioPackFormatID="AP_00011103" audioPackFormatName="5.1 + 6H (CICP 17)" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000F</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000E</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010010</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010012</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000C</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7.0 + 2H -->
        <audioPackFormat audioPackFormatID="AP_00011104" audioPackFormatName="7.0 + 2H" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000A</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000B</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001C</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010013</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010014</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7.0 (90 deg) -->
        <audioPackFormat audioPackFormatID="AP_00011105" audioPackFormatName="7.0 90-deg" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001C</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000A</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000B</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7.1 + 4H -->
        <!-- ChannelConfiguration 19 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_19 in MPEG-H ADM Profile 1.0.0 -->
        <!-- CICP 19 in MHAS 5.0.0 exports -->
        <audioPackFormat audioPackFormatID="AP_00011106" audioPackFormatName="7.1 + 4H (CICP 19)" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001C</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000A</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000B</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000F</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001E</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001F</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7.1 (60 deg) -->
        <!-- audioPackFormat_7 in MPEG-H ADM Profile 1.0.0 -->
        <audioPackFormat audioPackFormatID="AP_00011107" audioPackFormatName="7.1 60-deg" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010018</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010019</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7.1 (110 deg) -->
        <!-- ChannelConfiguration 12 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_12 in MPEG-H ADM Profile 1.0.0 -->
        <!-- CICP 12 in MHAS 5.0.0 exports -->
        <audioPackFormat audioPackFormatID="AP_00011108" audioPackFormatName="7.1 110-deg (CICP 12)" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001C</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001D</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- Quad -->
        <!-- ChannelConfiguration 10 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_10 in MPEG-H ADM Profile 1.0.0 -->
        <!-- CICP 10 in MHAS 5.0.0 exports -->
        <audioPackFormat audioPackFormatID="AP_00011109" audioPackFormatName="Quad (CICP 10)" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- Quad Height 110 -->
        <audioPackFormat audioPackFormatID="AP_0001110a" audioPackFormatName="Quad Height 110" typeLabel="0001">
          <audioChannelFormatIDRef>AC_0001000D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000F</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010010</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010012</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- Quad Height 135 -->
        <audioPackFormat audioPackFormatID="AP_0001110b" audioPackFormatName="Quad Height 135" typeLabel="0001">
          <audioChannelFormatIDRef>AC_0001000D</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000F</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001E</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001F</audioChannelFormatIDRef>
        </audioPackFormat>

        <!-- Layouts in MPEG-H ADM Profile 1.0.0 (but don't appear to be used in MHAS 5.0.0) -->

        <!-- 6/7.1 (6 Front, 7 Surr, 1 LFE) -->
        <!-- ChannelConfiguration 18 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_18 in MPEG-H ADM Profile 1.0.0 -->
        <audioChannelFormat audioChannelFormatID="AC_00011001" audioChannelFormatName="Left back surround" typeLabel="0001" typeDefinition="DirectSpeakers">
          <audioBlockFormat audioBlockFormatID="AB_00011001_00000001">
            <speakerLabel>CH_M_L150</speakerLabel>
            <position coordinate="azimuth">150.0</position>
            <position coordinate="elevation">0.0</position>
            <position coordinate="distance">1.0</position>
          </audioBlockFormat>
        </audioChannelFormat>
        <audioChannelFormat audioChannelFormatID="AC_00011002" audioChannelFormatName="Right back surround" typeLabel="0001" typeDefinition="DirectSpeakers">
          <audioBlockFormat audioBlockFormatID="AB_00011002_00000001">
            <speakerLabel>CH_M_R150</speakerLabel>
            <position coordinate="azimuth">-150.0</position>
            <position coordinate="elevation">0.0</position>
            <position coordinate="distance">1.0</position>
          </audioBlockFormat>
        </audioChannelFormat>
        <audioPackFormat audioPackFormatID="AP_00011012" audioPackFormatName="6/7.1 (CICP 18)" typeDefinition="DirectSpeakers" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010005</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010006</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00011001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00011002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000d</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000f</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000e</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010010</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010012</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000c</audioChannelFormatIDRef>
        </audioPackFormat>
        <!-- 7/6.1 (7 Front, 6 Surr, 1 LFE) -->
        <!-- ChannelConfiguration 20 (ISO/IEC 23091-3) -->
        <!-- audioPackFormat_20 in MPEG-H ADM Profile 1.0.0 -->
        <audioPackFormat audioPackFormatID="AP_00011014" audioPackFormatName="7/6.1 (CICP 20)" typeDefinition="DirectSpeakers" typeLabel="0001">
          <audioChannelFormatIDRef>AC_00010001</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010002</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010003</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010004</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001c</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001d</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000a</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001000b</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010022</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010023</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001e</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_0001001f</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010024</audioChannelFormatIDRef>
          <audioChannelFormatIDRef>AC_00010025</audioChannelFormatIDRef>
        </audioPackFormat>

      </audioFormatExtended>
    </format>
  </coreMetadata>
</ituADM>

)5UPPL3M3N74RY";
