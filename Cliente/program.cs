using System;

namespace Cliente
{
    class Program
    {
        static void Main(string[] args)
		{
			if (args.Length < 2)
			{
				Console.WriteLine("Uso: dotnet run <ip> <puerto>");
				return;
			}
			string ip = args[0];
			if (!int.TryParse(args[1], out int puerto))
			{
				Console.WriteLine("Puerto invalido.");
				return;
			}

			var chat = new Chat();
			chat.Iniciar(ip, puerto);
		}
    }
}