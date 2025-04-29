/**
 * LittleHelpers
 * A collection of some helper classes for various occasions.
 * https://github.com/bckmnn/littlehelpers
 *
 * Copyright (c) 2013 Stefan Beckmann http://bckmnn.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 * 
 * @author      Stefan Beckmann http://bckmnn.com
 * @modified    01/15/2014
 * @version     0.0.3 (3)
 */

package com.bckmnn.json;


import java.util.ArrayList;

import processing.data.JSONArray;
import processing.data.JSONObject;

/**
 * This class provides static methods to quickly get specific values out of a JSONObject.
 * It might have problems with getting an array (specific array values should be fine).
 * 
 * See the example for further information. 
 * 
 * @example JSONHelperExample
 *
 */

public abstract class JSONHelper {
	
	private static class WhatToGet {
		public TYPE type;
		public String name;
		public int index;
	}

	private static JSONObject jObject;
	private static JSONArray jArray;
	private static int jInt;
	private static String jString;
	private static float jFloat;
	private static long jLong;
	
	private static String currentPath;
	
	private static TYPE lastItem;

	public static boolean debug;
	
	private static enum TYPE{OBJECT,ARRAY,STRING,INT,LONG,FLOAT,DOUBLE,BOOL,ARRAY_ARRAY,ARRAY_OBJECT};
	
	public final static String VERSION = "0.0.3";
	
	public static int getInt(String path,JSONObject json) throws Exception{
		ArrayList<WhatToGet> list = parsePath(path, TYPE.INT);
		workList(list,json,TYPE.INT);
		return jInt;
	}
	
	public static long getLong(String path,JSONObject json) throws Exception{
		ArrayList<WhatToGet> list = parsePath(path, TYPE.LONG);
		workList(list,json,TYPE.LONG);
		return jLong;
	}
	
	public static String getString(String path,JSONObject json) throws Exception{
		ArrayList<WhatToGet> list = parsePath(path, TYPE.STRING);
		workList(list,json,TYPE.STRING);
		return jString;
	}
	
	public static float getFloat(String path,JSONObject json) throws Exception{
		ArrayList<WhatToGet> list = parsePath(path, TYPE.FLOAT);
		workList(list,json,TYPE.FLOAT);
		return jFloat;
	}
	
	private static void work(WhatToGet w) throws Exception{
		switch (w.type) {
		case INT:
			if(!workInt(w))
				throw new Exception("can't get int "+w.name);
			break;
		case ARRAY:
			if(!workArray(w))
				throw new Exception("can't get array "+w.name);
			break;
		case OBJECT:
			if(!workObject(w))
				throw new Exception("can't get object "+w.name);
			break;
		case STRING:
			if(!workString(w))
				throw new Exception("can't get string "+w.name);
			break;
		case FLOAT:
			if(!workFloat(w))
				throw new Exception("can't get string "+w.name);
			break;
		case LONG:
			if(!workLong(w))
				throw new Exception("can't get string "+w.name);
			break;
		default:
			throw new Exception("sorry "+w.type+" not implemented");
	}
	}
	

	private static void workList(ArrayList<WhatToGet> list, JSONObject json, TYPE t) throws Exception {
		lastItem = TYPE.OBJECT;
		jObject = json;
		if(debug)
			System.out.println("-------- getting from object");
		for (int i = 0; i < list.size(); i++) {
			WhatToGet w = list.get(i);
			try {
				work(w);
				lastItem = w.type;
				if(i < list.size()-1){
					if(w.type == TYPE.ARRAY){
						WhatToGet next = list.get(i+1);
						if(!(next.type == TYPE.ARRAY && next.name.equals(""))){
							workObject(w);
							lastItem = TYPE.OBJECT;
						}
					}
				}else{
					if(w.type == TYPE.ARRAY){
						lastItem = TYPE.ARRAY;
						w.type = t;
						work(w);
					}
				}
				if(debug)
					System.out.println("  property "+w.name+"/"+w.index+" as "+ w.type+"["+lastItem+"]");
			} catch (Exception e) {
				System.err.println("[JSONHelper] "+e);
				System.err.println("[JSONHelper] Exception at segment "+(i+1)+" in Path: "+currentPath);
				throw new Exception(" there was an error fetching property "+w.name+"/"+w.index+" at segment "+(i+1)+" as "+ w.type+"["+lastItem+"]");
			}
			
		}
	}

	private static boolean workLong(WhatToGet w) {
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jLong = jObject.getLong(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jLong = jArray.getLong(w.index);
					return true;
				}
				break;
		}
		return false;
	}
	
	private static boolean workFloat(WhatToGet w) {
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jFloat = jObject.getFloat(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jFloat = jArray.getFloat(w.index);
					return true;
				}
				break;
		}
		return false;
	}

	private static boolean workString(WhatToGet w) {
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jString = jObject.getString(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jString = jArray.getString(w.index);
					return true;
				}
				break;
		}
		return false;
	}

	private static boolean workObject(WhatToGet w) {
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jObject = jObject.getJSONObject(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jObject = jArray.getJSONObject(w.index);
					return true;
				}
				break;
		}
		return false;
	}

	private static boolean workArray(WhatToGet w) {
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jArray = jObject.getJSONArray(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jArray = jArray.getJSONArray(w.index);
					return true;
				}
				break;
		}
		return false;
	}

	private static boolean workInt(WhatToGet w){
		switch (lastItem) {
			case OBJECT:
				if(checkObject(w.name)){
					jInt = jObject.getInt(w.name);
					return true;
				}
				break;
			case ARRAY:
				if(checkArray(w.index)){
					jInt = jArray.getInt(w.index);
					return true;
				}
				break;
		}
		return false;
	}
	

	private static boolean checkArray(int index) {
		if(index >= 0  && index < jArray.size()){
			return true;
		}
		return false;
	}

	private static boolean checkObject(String name) {
		if(jObject.hasKey(name)){
			return true;
		}else{
			
		}
		return false;
	}

	private static ArrayList<WhatToGet> parsePath(String path, TYPE t) throws Exception{
		currentPath = path;
		ArrayList<WhatToGet> list = new ArrayList<JSONHelper.WhatToGet>(); 
		String[] elements = path.split("\\.");
		if(elements.length == 0)
			throw new Exception("path is empty");
		for (int i = 0; i < elements.length; i++) {
			WhatToGet w = new WhatToGet();
			if(elements[i].contains("[") && elements[i].contains("]")){
				// looking for an array
				String[] arrayInfo = elements[i].split("(\\[{1})|(\\]{1})");
				if(arrayInfo.length == 2){
					try {
						w.name = arrayInfo[0];
						w.index = Integer.valueOf(arrayInfo[1]);
						w.type = TYPE.ARRAY;
						list.add(w);
					} catch (Exception e) {
						throw new Exception("there is an error in your array index at path segment "+(i+1)+": "+elements[i]);
					}
				}else if(arrayInfo.length == 1){
					try {
						w.name = "";
						w.index = Integer.valueOf(arrayInfo[1]);
						w.type =  TYPE.ARRAY;
						list.add(w);
					} catch (Exception e) {
						throw new Exception("there is an error in your array index at path segment "+(i+1)+": "+elements[i]);
					}
				}else{
					throw new Exception("there is an error in your path at segment "+(i+1)+": "+elements[i]);
				}
			}else{
				w.name = elements[i];
				w.index = -1;
				if(i<elements.length-1){
					w.type = TYPE.OBJECT;
				}else{
					w.type = t;
				}
				list.add(w);
			}
		}
		return list;
	}

	/**
	 * return the version of the library.
	 * 
	 * @return String
	 */
	public static String version() {
		return VERSION;
	}
}

