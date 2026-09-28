using System;
using System.Collections.Generic;
using System.Text.Json;

namespace Cliente
{
	public class AnalizarComando
	{
		public List<string> ProcesarEntradaUsuario(string input)
		{
			var mensajesJSON = new List<string>();
			string[] partes = input.Split(' ', StringSplitOptions.RemoveEmptyEntries);

			if (partes.Length == 0) return mensajesJSON;

			string comando = partes[0].ToLower();

			try
			{
				switch (comando)
				{
					case "/estado":
						if (partes.Length < 2)
						{
							Console.WriteLine("Uso: /estado <AWAY|BUSY|ACTIVE");
							break;
						}
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "STATUS", status = partes[1].ToUpper() }));
						break;

					case "/usuarios":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "USERS" }));
						break;

					case "/privado":
						if (partes.Length < 3)
						{
							Console.WriteLine("Uso: /privado <usuario> <texto>");
							break;
						}
						string mensajeTexto = string.Join(" ", partes, 2, partes.Length -2);
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "TEXT", username = partes[1], text = mensajeTexto}));
						break;

					case "/publico":
						if (partes.Length < 2) 
						{
							Console.WriteLine("Uso: /publico <texto>");
							break;
						}
						string textoPublico = string.Join(" ", partes, 1, partes.Length - 1);
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "PUBLIC_TEXT", text = textoPublico}));
						break;

					case "/nuevasala":
						if (partes.Length < 2)
						{
							Console.WriteLine("Uso: /nuevasala <sala>");
							break;
						}
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "NEW_ROOM", roomname = partes[1]}));
						break;

					case "/invitar":
						if (partes.Length < 3)
						{
							Console.WriteLine("Uso: /invitar <sala> <usuario> [<usuario2>...]");
							break;
						}
						for (int i = 2; i < partes.Length; i++)
						{
							mensajesJSON.Add(JsonSerializer.Serialize(new { type = "INVITE", roomname = partes[1], username = partes[i]}));
						}
						break;

					case "/salausuarios":
						if (partes.Length < 2)
						{
							Console.WriteLine("Uso: /salausuarios <sala>");
							break;
						}
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "ROOM_USERS", roomname = partes[1] }));
						break;

					case "/mensajesala":
						if (partes.Length < 3)
						{
							Console.WriteLine("Uso: /mensajesala <sala> <texto>");
							break;
						}
						string mensajeSala = string.Join(" ", partes, 2, partes.Length -2);
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "ROOM_TEXT", roomname = partes[1], text = mensajeSala }));
						break;

					case "/irse":
						if (partes.Length < 2)
						{
							Console.WriteLine("Uso: /irse <sala>");
							break;
						}
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "LEAVE_ROOM", roomname = partes[1] }));
						break;

					case "/cerrar":
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "DISCONNECT" }));
						break;

					case "/ayuda":
						AyudaVisual();
						break;
					
					default:
						Console.WriteLine("Comando no reconocido. Usa /ayuda para mostrar los comandos.");
						break;
				}
			}
			catch (Exception ex)
			{
				Console.WriteLine($"Error al procesar el comando: {ex.Message}");
			}

			return mensajesJSON;
		}

		public void ProcesarMensajeServidor(string json)
		{
			try
			{
				using var doc = JsonDocument.Parse(json);
				var raiz = doc.RootElement;
		
				if (!raiz.TryGetProperty("type", out var tipoE)) return;
				string tipo = tipoE.GetString() ?? "";

				switch (tipo)
				{
					case "RESPONSE":
						Console.ForegroundColor = ConsoleColor.DarkGray;
						string op = raiz.GetProperty("operation").GetString() ?? "";
						string res = raiz.GetProperty("result").GetString() ?? "";
						string extra = raiz.TryGetProperty("extra", out var extE) ? extE.GetString() ?? "" : "";
						Console.WriteLine($"[SERVIDOR] Respuesta a {op}: {res} {extra}");
						Console.ResetColor();
						break;

					case "NEW_USER":
						Console.ForegroundColor = ConsoleColor.Cyan;
						Console.WriteLine($"[SISTEMA] El usuario {raiz.GetProperty("username").GetString()} se ha conectado.");
						Console.ResetColor();
						break;

					case "NEW_STATUS":
						Console.ForegroundColor = ConsoleColor.Cyan;
						Console.WriteLine($"[SISTEMA] {raiz.GetProperty("username").GetString()} ahora esta {raiz.GetProperty("status").GetString()}.");
						Console.ResetColor();
						break;

					case "USER_LIST":
						Console.ForegroundColor = ConsoleColor.Cyan;
						Console.WriteLine("[SISTEMA] Usuarios conectados:");
						var usuarios = raiz.GetProperty("users").EnumerateObject();
						foreach (var usuario in usuarios)
						{
							Console.WriteLine($"  - {usuario.Name}: {usuario.Value.GetString()}");
						}
						Console.ResetColor();
						break;

					case "TEXT_FROM":
						Console.ForegroundColor = ConsoleColor.Magenta;
						Console.WriteLine($"[PRIVADO de {raiz.GetProperty("username").GetString()}]: {raiz.GetProperty("text").GetString()}");
						Console.ResetColor();
						break;

					case "PUBLIC_TEXT_FROM":
						Console.ForegroundColor = ConsoleColor.Yellow;
						Console.WriteLine($"[PÚBLICO de {raiz.GetProperty("username").GetString()}]: {raiz.GetProperty("text").GetString()}");
						Console.ResetColor();
						break;

					case "INVITATION":
						Console.ForegroundColor = ConsoleColor.Cyan;
						Console.WriteLine($"[SISTEMA] {raiz.GetProperty("username").GetString()} te ha invitado a la sala '{raiz.GetProperty("roomname").GetString()}'.");
						Console.ResetColor();
						break;

					case "JOINED_ROOM":
						Console.ForegroundColor = ConsoleColor.Green;
						Console.WriteLine($"[SALA {raiz.GetProperty("roomname").GetString()}] {raiz.GetProperty("username").GetString()} se ha unido.");
						Console.ResetColor();
						break;

					case "ROOM_USER_LIST":
						Console.ForegroundColor = ConsoleColor.Green;
						Console.WriteLine($"[SALA {raiz.GetProperty("roomname").GetString()}] Usuarios en la sala:");
						var usuariosSala = raiz.GetProperty("users").EnumerateObject();
						foreach (var usuario in usuariosSala)
						{
							Console.WriteLine($"  - {usuario.Name}: {usuario.Value.GetString()}");
						}
						Console.ResetColor();
						break;

					case "ROOM_TEXT_FROM":
						Console.ForegroundColor = ConsoleColor.Green;
						Console.WriteLine($"[SALA {raiz.GetProperty("roomname").GetString()} - {raiz.GetProperty("username").GetString()}]: {raiz.GetProperty("text").GetString()}");
						Console.ResetColor();
						break;

					case "LEFT_ROOM":
						Console.ForegroundColor = ConsoleColor.Green;
						Console.WriteLine($"[SALA {raiz.GetProperty("roomname").GetString()}] {raiz.GetProperty("username").GetString()} ha salido.");
						Console.ResetColor();
						break;

					case "DISCONNECTED":
						Console.ForegroundColor = ConsoleColor.Cyan;
						Console.WriteLine($"[SISTEMA] El usuario {raiz.GetProperty("username").GetString()} se ha desconectado.");
						Console.ResetColor();
						break;

					default:
						Console.WriteLine($"[DESCONOCIDO] {json}");
						break;
				}
			}
			catch
			{
			}
		}

		public void AyudaVisual()
		{
			Console.WriteLine("--- Comandos del Cliente ---");
			Console.WriteLine("/estado <AWAY|BUSY|ACTIVE>		- Cambia tu estado");
			Console.WriteLine("/usuarios						- Lista los usuarios conectados");
			Console.WriteLine("/privado <usuario> <texto>		- Envia un mensaje privado");
			Console.WriteLine("/publico <texto>					- Envia un mensaje publico");
			Console.WriteLine("/nuevasala <sala>				- Creacion de sala nueva");
			Console.WriteLine("/invitar <sala> <usuario>		- Invita a alguien a la sala");
			Console.WriteLine("/salausuarios <sala>				- Lista los usuarios de la sala");
			Console.WriteLine("/mensajesala <sala> <texto>		- Envia un mensaje en la sala");
			Console.WriteLine("/irse <texto>					- Abandona la sala");
			Console.WriteLine("/cerrar							- Desconexion y cierre del cliente");
			Console.WriteLine("/ayuda							- Muestra los comandos");
			Console.WriteLine("----------------------------");
		}

		public string GenerarIdentificacion(string username)
		{
			return JsonSerializer.Serialize(new { type = "IDENTIFY", username = username});
		}
	}
}