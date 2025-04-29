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

package com.bckmnn.udp;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Method;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.util.Enumeration;

import processing.core.PApplet;

/**
 * This class helps setting up a simple UDP communication. 
 *
 * @example UDPHelperExample
 */

public class UDPHelper {

	public final static String VERSION = "0.0.3";

	private DatagramChannel channel;
	private DatagramSocket socket;
	private SocketAddress address;

	private int messageBufferSize = 512;

	private ByteBuffer messageBuffer;

	private int localPort = 13370;

	private PApplet p;

	private Method onMessageRecievedMethod, registerMethod;

	public UDPHelper(PApplet theParent) {
		messageBuffer = ByteBuffer.allocate(messageBufferSize);

		p = theParent;
		try {
			registerMethod = p.getClass().getMethod("registerMethod",
					new Class[] { String.class, Object.class });
			registerMethod.invoke(p, "dispose", this);
		} catch (Exception e) {
			System.out
					.println("[UDPHelper] please call UDPHelper.dispose() in PApplet.exit()");
		}
		
		try{
			registerMethod = p.getClass().getMethod("registerMethod",
					new Class[] { String.class, Object.class });
			registerMethod.invoke(p, "pre", this);
		} catch (Exception e) {
			System.out
					.println("[UDPHelper] please call UDPHelper.run() in PApplet.draw()");
		}

		onMessageRecievedMethod = null;

		// check to see if the host applet implements
		// public void onUdpMessageRecieved()
		try {
			onMessageRecievedMethod = p.getClass().getMethod(
					"onUdpMessageRecieved",
					new Class[] { SocketAddress.class, byte[].class });
		} catch (Exception e) {
			// no such method
			System.err
					.println("[UDPHelper] could not find method public void onUdpMessageRecieved (SocketAddress client, byte[] message)");
		}

		try {
			System.out.println("[UDPHelper] list of network interfaces:");
			Enumeration<NetworkInterface> en = NetworkInterface
					.getNetworkInterfaces();
			if (en != null) {
				for (Enumeration<NetworkInterface> e = en; e.hasMoreElements();) {
					NetworkInterface intf = e.nextElement();
					System.out.println("    " + intf.getName() + " "
							+ intf.getDisplayName());
					for (Enumeration<InetAddress> enumIpAddr = intf
							.getInetAddresses(); enumIpAddr.hasMoreElements();) {
						System.out.println("        "
								+ enumIpAddr.nextElement().toString());
					}
				}
			}
		} catch (SocketException e) {
			System.err.println("[UDPHelper] (error retrieving network interface list)");
		}
	}

	/**
	 * Initializes the socket and channel etc. needs to be called once during setup.
	 * Binds the socket to the specified port. 
	 */
	public void startListening() {
		try {
			channel = DatagramChannel.open();
			channel.configureBlocking(false);
			socket = channel.socket();
			socket.setBroadcast(true);
			address = new InetSocketAddress(localPort);
			socket.bind(address);
		} catch (Exception e) {
			System.err
					.println("[UDPHelper] ERROR listening at port "
							+ localPort
							+ ".\n It might be that another application is already listening on this port.");
			e.printStackTrace();
			return;
		}
	}

	/**
	 * sends a message to the reciever (client)
	 * @param message udp message
	 * @param client the receivers address
	 */
	public void sendMessage(byte[] message, SocketAddress client) {
		if (channel != null && channel.isOpen()) {
			try {
				ByteBuffer buffer = ByteBuffer.wrap(message);
				channel.send(buffer, client);
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	/**
	 * converts a string into a byte array.
	 * @param string input string
	 * @return string encoded as a byte array
	 */
	public static byte[] bytesFromString(String string) {
		return string.getBytes();
	}
	

	/**
	 * converts a byte array into an String
	 * @param bytes input byte array
	 * @return output string
	 */
	public static String stringFromBytes(byte[] bytes) {
		String string = "";
		try {
			string = new String(bytes, "UTF-8");
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		}
		return string;
	}

	
	/**
	 * stops listening on the port, closes the socket.
	 */
	public void endListening() {
		try {
			channel.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		channel = null;
		socket = null;
	}

	public void dispose() {
		try {
			channel.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 *  the run method takes care of reading messages from the socket into the buffer
	 *  and calls void onUdpMessageRecieved (SocketAddress client, byte[] message) in your app.
	 *  In Processing 2.1+ this method is called automatically before draw(). 
	 */
	public void run() {
		if (channel.isOpen()) {
			try {
				messageBuffer.clear();
				SocketAddress client = channel.receive(messageBuffer);
				messageBuffer.flip();
				if (messageBuffer.hasRemaining()) {
					byte[] bytes = new byte[messageBuffer.remaining()];
					for (int i = 0; i < bytes.length; i++) {
						bytes[i] = messageBuffer.get(i);
					}
					if (onMessageRecievedMethod != null) {
						onMessageRecievedMethod.invoke(p, client, bytes);
					}
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
	
	/**
	 * wraps run method to be called automatically by Processing 2.1+
	 */
	public void pre(){
		run();
	}
	
	public int getMessageBufferSize() {
		return messageBufferSize;
	}

	/**
	 * sets the size of the message buffer in bytes
	 * @param messageBufferSize
	 */
	public void setMessageBufferSize(int messageBufferSize) {
		this.messageBufferSize = messageBufferSize;
		messageBuffer = ByteBuffer.allocate(messageBufferSize);
	}

	public int getLocalPort() {
		return localPort;
	}

	/**
	 * sets the local port number. thats the port number your application will listen on.
	 * @param localPort
	 */
	public void setLocalPort(int localPort) {
		this.localPort = localPort;
	}

	/**
	 * returns the version of the library.
	 * 
	 * @return String
	 */
	public static String version() {
		return VERSION;
	}

}
