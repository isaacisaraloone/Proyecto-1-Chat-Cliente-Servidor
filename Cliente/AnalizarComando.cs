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
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "STATUS", estado = partes[1].ToUpper() }));
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
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "TEXT", username = partes[1], texto = mensajeTexto}));
						break;

					case "/publico":
						if (partes.Length < 2) 
						{
							Console.WriteLine("Uso: /publico <texto>");
							break;
						}
						string textoPublico = string.Join(" ", partes, 1, partes.Length - 1);
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "PUBLIC_TEXT", texto = textoPublico}));
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
						mensajesJSON.Add(JsonSerializer.Serialize(new { type = "ROOM_TEXT", roomname = partes[1], texto = mensajeSala }));
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