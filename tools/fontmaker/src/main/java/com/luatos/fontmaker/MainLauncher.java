package com.luatos.fontmaker;

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.nutz.lang.Encoding;
import org.nutz.lang.Files;
import org.nutz.lang.Lang;
import org.nutz.log.Log;
import org.nutz.log.Logs;

// ���FontMakerҲ��¡���� https://gitee.com/kerndev/FontMaker.git
/*
������Ǹ�ɶ��? ���FontMakerά��/�����ֿ�c�Ͷ�Ӧ��map�ļ�.
��Ҫ��ϵ��������:
1. FontMaker����cst�ļ������ֿ�c�ļ�.
2. FontMaker�Դ���cst�����е�ȱ��, �����ṩ����չ�ķ�����.
3. FontMaker�����ֿ�.c�ļ���, ��һ��������,�ڶ�����map,ʵ�����޷�����,��Ҫɾ����, Ȼ���ñ����main1��������map.
4. �ϸ���˵�ֿ�.c�ļ��ܱ���,�����������,�޷�ʹ�ö��ַ����в���, ����������map���Խ���������.
 */
public class MainLauncher {

	private static final Log log = Logs.get();
	
	public static void main(String[] args) throws Exception{
		// ȡ��ע����ִ����ع���
		//main1(args);
		//main2(args);
		//main4(args);
		log.info("Good Day");
	}

	// ���� chinese_gb2312_16p.c . ʹ��FontMakerѡ��������ֺź�,������ļ�����.c�ļ�,��ʽ����
	/*
const unsigned char  char_bits_gb2312_16p[][32]=
{
//U+2500(��)
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFC,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	 */
	// ���µĴ�����ǰ���//U+���������ֱ���, Ȼ��ó�ӳ���chinese_map
	public static void main1(String[] args) throws Exception {
		FileReader fr = new FileReader("D:\\github\\LuatOS\\luat\\packages\\fonts\\chinese_gb2312_16p.c", Encoding.CHARSET_UTF8);
		BufferedReader br = new BufferedReader(fr);
		StringBuilder sb = new StringBuilder("static const uint16_t fontmap[7571]={");
		int count = 0;
		while (br.ready()) {
			String line = br.readLine();
			if (line == null)
				break;
			line = line.trim();
			if (line.startsWith("//U+")) {
				int val = Integer.parseInt(line.substring(4, 8), 16);
				if (count % 16 == 0)
					sb.append("\r\n\t");
				sb.append("0x");
				sb.append(Lang.fixedHexString(new byte[] {(byte) (val & 0xFF), (byte) (val >> 8)}));
				sb.append(",");
				count ++;
			}
		}
		sb.setCharAt(sb.length() - 1, '}');
		sb.append(";\r\n");
		System.out.println(sb);
		System.out.println("count=" + count);
		Files.write("D://gb2312_map.c", sb);
		br.close();
	}
	
	/*
	 * �����������FontMakerԭ�е�GB2312������, ��ԭ�����ݽ�������,�ó������GB2312_fix.cst
	 */
	public static void main2(String[] args) throws Exception {
		File f = new File("D:/github/FontMaker/bin/charset/GB2312.cst");
		DataInputStream ins = new DataInputStream(new FileInputStream(f));
		DataOutputStream out = new DataOutputStream(new FileOutputStream("D:/github/FontMaker/bin/charset/GB2312_fix.cst"));
		List<Integer> unicodes = new ArrayList<>(9000);
		
		// ����, ��ASCII����
		
		while (ins.available() > 0) {
			unicodes.add(ins.readUnsignedShort());
		}
		Collections.sort(unicodes);
		
		for (int i = 1; i <= 0x7F; i++) {
			out.writeShort(i);
		}
		for (Integer val : unicodes) {
			System.out.println(Integer.toHexString(val));
			out.writeShort(val);
		}
		System.out.println(unicodes.size());
		out.flush();
		out.close();
		ins.close();
	}
	
	// ��ȡgb2312k����ı���, Ȼ����ASIIC��,��������gb2312k2.cst
	// �ó���gb2312k2.cst��FontMaker�����ֿ�c�ļ�
	public static void main4(String[] args) throws Exception {
		DataOutputStream out = new DataOutputStream(new FileOutputStream("D://github/FontMaker/bin/charset/gb2312k2.cst"));
		BufferedReader br = new BufferedReader(new FileReader(new File("D://github/FontMaker/doc/gb2312k.txt")));
		List<Integer> vals = new ArrayList<>();
		while (br.ready()) {
			String line = br.readLine();
			if (line == null)
				break;
			if (line.length() == 0)
				continue;
			int val = Integer.parseInt(line, 16);
			if (val < 0x7E)
				continue;
			//out.writeShort(val);
			int H = (val >> 8) & 0xFF;
			int L = val & 0xFF;
			val = (L << 8) + H;
			vals.add(val);
		}

		// ����ASCII�ַ�
		for (int i = 1; i <= 0x7E; i++) {
			vals.add(i << 8);
		}
		
		Collections.sort(vals);
		
		
		for (Integer val : vals) {
			//out.writeByte((val & 0xFF));
			//out.writeByte(((val >> 8) & 0xFF));
			out.writeShort(val);
		}
		out.flush();
		out.close();
	}

}
